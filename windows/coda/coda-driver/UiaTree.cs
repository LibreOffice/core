// -*- tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// The UI Automation side of coda-driver: finding elements in the windows of the target process,
// acting on them, and describing the windows as XML.
//
// The roots are the visible top-level windows of the target process, in z-order, so a dialog that
// is in front of a document window is searched first. The common file dialogs run inside the
// process that shows them, so their windows are among these.
//
// The content of a web view is left out of every walk, as in macos/coda-driver, because the pages
// are driven through msedgedriver, and their trees are large.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Automation;

namespace CodaDriver
{
    class UiaTree
    {
        public record Predicate(string Attribute, string Value);

        public class Found
        {
            public List<AutomationElement> Elements { get; } = new();

            // The walk stopped at the time limit, before the whole tree was seen.
            public bool TimedOut { get; set; }
        }

        // The longest time that one walk of the tree takes.
        public const int WalkSeconds = 5;

        readonly int targetPid;

        public UiaTree(int targetPid)
        {
            this.targetPid = targetPid;
        }

        // Finding

        public Found Find(Predicate predicate, bool firstOnly)
        {
            var found = new Found();
            var clock = Stopwatch.StartNew();
            var queue = new Queue<AutomationElement>(TopLevelWindows());

            while (queue.Count > 0)
            {
                if (clock.Elapsed.TotalSeconds > WalkSeconds)
                {
                    found.TimedOut = true;
                    break;
                }

                AutomationElement element = queue.Dequeue();
                try
                {
                    if (Matches(element, predicate))
                    {
                        found.Elements.Add(element);
                        if (firstOnly)
                            break;
                    }
                    foreach (AutomationElement child in ChildrenToWalk(element))
                        queue.Enqueue(child);
                }
                catch (ElementNotAvailableException)
                {
                    // The element went away during the walk, for instance a dialog that closed.
                }
            }

            return found;
        }

        static bool Matches(AutomationElement element, Predicate predicate)
        {
            AutomationElement.AutomationElementInformation current = element.Current;
            switch (predicate.Attribute)
            {
                case "accessibility-id":
                case "identifier":
                    return current.AutomationId == predicate.Value;
                case "name":
                case "title":
                    return current.Name == predicate.Value;
                case "role":
                    return RoleOf(current) == predicate.Value;
                case "class":
                    return current.ClassName == predicate.Value;
                default:
                    return false;
            }
        }

        List<AutomationElement> TopLevelWindows()
        {
            var handles = new List<IntPtr>();
            EnumWindows((hWnd, lParam) =>
                {
                    GetWindowThreadProcessId(hWnd, out uint pid);
                    if (pid == targetPid && IsWindowVisible(hWnd))
                        handles.Add(hWnd);
                    return true;
                },
                IntPtr.Zero);

            var windows = new List<AutomationElement>();
            foreach (IntPtr hWnd in handles)
            {
                try
                {
                    windows.Add(AutomationElement.FromHandle(hWnd));
                }
                catch (ElementNotAvailableException)
                {
                }
            }
            return windows;
        }

        // The children of an element, except those of a web view.
        static List<AutomationElement> ChildrenToWalk(AutomationElement element)
        {
            var children = new List<AutomationElement>();
            AutomationElement.AutomationElementInformation current = element.Current;
            if (current.FrameworkId == "Chrome" || current.ClassName.StartsWith("Chrome_"))
                return children;

            TreeWalker walker = TreeWalker.ControlViewWalker;
            for (AutomationElement? child = walker.GetFirstChild(element); child != null;
                 child = walker.GetNextSibling(child))
                children.Add(child);
            return children;
        }

        // Actions

        public static void Click(AutomationElement element)
        {
            AutomationElement.AutomationElementInformation current = element.Current;
            if (!current.IsEnabled)
                throw new WebDriverException("element not interactable",
                                             $"{Describe(current)} is disabled");

            // A button that is a window of its own is clicked the way the button itself reports a
            // click: with WM_COMMAND and BN_CLICKED to its parent. This works whether the dialog
            // is active or not.
            var hWnd = new IntPtr(current.NativeWindowHandle);
            if (current.ControlType == ControlType.Button && hWnd != IntPtr.Zero)
            {
                IntPtr parent = GetParent(hWnd);
                int controlId = GetDlgCtrlID(hWnd);
                if (parent != IntPtr.Zero && controlId != 0)
                {
                    var wParam = new IntPtr((BN_CLICKED << 16) | (controlId & 0xFFFF));
                    PostMessage(parent, WM_COMMAND, wParam, hWnd);
                    return;
                }
            }

            if (element.TryGetCurrentPattern(InvokePattern.Pattern, out object pattern))
            {
                // Invoke() returns only when the action does, and an action that shows a modal
                // dialog takes as long as the dialog stays up. So the request is answered after
                // a short wait, whether the action has finished or not.
                var invoke = (InvokePattern)pattern;
                Task.Run(invoke.Invoke).Wait(TimeSpan.FromSeconds(2));
                return;
            }

            throw new WebDriverException("element not interactable",
                                         $"{Describe(current)} cannot be clicked");
        }

        public static void SetValue(AutomationElement element, string text)
        {
            if (!element.TryGetCurrentPattern(ValuePattern.Pattern, out object pattern))
                throw new WebDriverException("invalid element state",
                                             $"{Describe(element.Current)} takes no value");
            ((ValuePattern)pattern).SetValue(text);
        }

        // An element that takes no value has nothing to clear, so clearing it does nothing, as in
        // macos/coda-driver.
        public static void Clear(AutomationElement element)
        {
            if (element.TryGetCurrentPattern(ValuePattern.Pattern, out object pattern)
                && !((ValuePattern)pattern).Current.IsReadOnly)
                ((ValuePattern)pattern).SetValue("");
        }

        // Describing

        // The windows of the target process as XML, in the shape of the AT-SPI page source that
        // the shared specs know from CODA-Q: a desktop_frame root, element names from the AT-SPI
        // roles, and the attributes name, role, description, accessibility-id and path. Each
        // element also has its window class, when it has one, as class.
        public string PageSource()
        {
            var clock = Stopwatch.StartNew();
            var output = new StringBuilder();
            output.Append("<desktop_frame name=\"main\" role=\"\" description=\"\" path=\"\">\n");

            string processName;
            try
            {
                processName = Process.GetProcessById(targetPid).ProcessName;
            }
            catch (ArgumentException)
            {
                processName = "";
            }
            output.Append($"  <application name=\"{Escape(processName)}\" role=\"\""
                          + " description=\"\" path=\"0\">\n");

            List<AutomationElement> windows = TopLevelWindows();
            for (int i = 0; i < windows.Count; i++)
                Serialize(windows[i], "0 " + i, 2, clock, output);

            if (clock.Elapsed.TotalSeconds > WalkSeconds)
                output.Append($"  <!-- the walk stopped after {WalkSeconds} seconds;"
                              + " elements are missing -->\n");

            output.Append("  </application>\n");
            output.Append("</desktop_frame>\n");
            return output.ToString();
        }

        static void Serialize(AutomationElement element, string path, int depth, Stopwatch clock,
                              StringBuilder output)
        {
            AutomationElement.AutomationElementInformation current;
            List<AutomationElement> children;
            try
            {
                current = element.Current;
                children = clock.Elapsed.TotalSeconds > WalkSeconds
                    ? new List<AutomationElement>()
                    : ChildrenToWalk(element);
            }
            catch (ElementNotAvailableException)
            {
                return;
            }

            string tag = TagOf(element, current);
            string indent = new string(' ', 2 * depth);
            output.Append($"{indent}<{tag} name=\"{Escape(current.Name)}\""
                          + $" role=\"{Escape(RoleOf(current))}\""
                          + $" description=\"{Escape(current.HelpText)}\"");
            if (current.AutomationId != "")
                output.Append($" accessibility-id=\"{Escape(current.AutomationId)}\"");
            if (current.ClassName != "")
                output.Append($" class=\"{Escape(current.ClassName)}\"");
            output.Append($" path=\"{path}\"");

            if (children.Count == 0)
            {
                output.Append("/>\n");
                return;
            }
            output.Append(">\n");
            for (int i = 0; i < children.Count; i++)
            {
                if (clock.Elapsed.TotalSeconds > WalkSeconds)
                    break;
                Serialize(children[i], path + " " + i, depth + 1, clock, output);
            }
            output.Append($"{indent}</{tag}>\n");
        }

        // The control type without its "ControlType." prefix, for example "Button".
        static string RoleOf(AutomationElement.AutomationElementInformation current)
        {
            string name = current.ControlType.ProgrammaticName;
            return name.StartsWith("ControlType.") ? name.Substring("ControlType.".Length) : name;
        }

        // The AT-SPI role names of the UI Automation control types. A type that is not listed
        // becomes its own name in snake case, so for example SplitButton becomes split_button.
        static readonly Dictionary<ControlType, string> TagOfControlType = new()
        {
            [ControlType.Button] = "push_button",
            [ControlType.CheckBox] = "check_box",
            [ControlType.ComboBox] = "combo_box",
            [ControlType.Custom] = "panel",
            [ControlType.DataGrid] = "table",
            [ControlType.DataItem] = "table_cell",
            [ControlType.Document] = "document_frame",
            [ControlType.Edit] = "text",
            [ControlType.Group] = "panel",
            [ControlType.Hyperlink] = "link",
            [ControlType.Pane] = "panel",
            [ControlType.RadioButton] = "radio_button",
            [ControlType.Tab] = "page_tab_list",
            [ControlType.TabItem] = "page_tab",
            [ControlType.Text] = "label",
            [ControlType.ToolBar] = "tool_bar",
        };

        static string TagOf(AutomationElement element,
                            AutomationElement.AutomationElementInformation current)
        {
            if (current.ControlType == ControlType.Window)
            {
                // A dialog box, or a window that blocks its owner, is a dialog; any other window
                // is a frame.
                if (current.ClassName == "#32770")
                    return "dialog";
                if (element.TryGetCurrentPattern(WindowPattern.Pattern, out object pattern)
                    && ((WindowPattern)pattern).Current.IsModal)
                    return "dialog";
                return "frame";
            }

            if (current.ControlType == ControlType.Document && current.FrameworkId == "Chrome")
                return "document_web";

            if (TagOfControlType.TryGetValue(current.ControlType, out string? tag))
                return tag;

            var snakeCase = new StringBuilder();
            foreach (char c in RoleOf(current))
            {
                if (char.IsUpper(c) && snakeCase.Length > 0)
                    snakeCase.Append('_');
                snakeCase.Append(char.ToLowerInvariant(c));
            }
            return snakeCase.Length > 0 ? snakeCase.ToString() : "accessible";
        }

        static string Describe(AutomationElement.AutomationElementInformation current)
        {
            return $"{RoleOf(current)} \"{current.Name}\" (id \"{current.AutomationId}\")";
        }

        static string Escape(string text)
        {
            var result = new StringBuilder(text.Length);
            foreach (char c in text)
            {
                switch (c)
                {
                    case '&': result.Append("&amp;"); break;
                    case '<': result.Append("&lt;"); break;
                    case '>': result.Append("&gt;"); break;
                    case '"': result.Append("&quot;"); break;
                    default:
                        // XML 1.0 has no way to write most control characters.
                        if (c >= 0x20 || c == '\t' || c == '\n' || c == '\r')
                            result.Append(c);
                        break;
                }
            }
            return result.ToString();
        }

        // Win32

        const uint WM_COMMAND = 0x0111;
        const int BN_CLICKED = 0;

        delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

        [DllImport("user32.dll")]
        static extern bool EnumWindows(EnumWindowsProc callback, IntPtr lParam);

        [DllImport("user32.dll")]
        static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);

        [DllImport("user32.dll")]
        static extern bool IsWindowVisible(IntPtr hWnd);

        [DllImport("user32.dll")]
        static extern IntPtr GetParent(IntPtr hWnd);

        [DllImport("user32.dll")]
        static extern int GetDlgCtrlID(IntPtr hWnd);

        [DllImport("user32.dll")]
        static extern bool PostMessage(IntPtr hWnd, uint message, IntPtr wParam, IntPtr lParam);
    }
}
