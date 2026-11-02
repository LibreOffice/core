// -*- tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// The HTTP side of coda-driver. It answers this part of the W3C WebDriver protocol, the same part
// that macos/coda-driver answers:
//
//   GET    /status
//   POST   /session
//   DELETE /session/{id}
//   POST   /session/{id}/element
//   POST   /session/{id}/elements
//   POST   /session/{id}/element/{eid}/click
//   POST   /session/{id}/element/{eid}/value
//   POST   /session/{id}/element/{eid}/clear
//   GET    /session/{id}/element/{eid}/displayed
//   GET    /session/{id}/source
//
// A selector is an XPath of the form //*[@attribute="value"] and nothing else.
//
// The server takes connections from the local machine only, one at a time, and closes each after
// one response. A plain TCP listener does the HTTP, so no URL reservation is needed, as it would
// be for HttpListener.

using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Windows.Automation;

namespace CodaDriver
{
    // A request that could not be carried out, as the WebDriver error it is answered with.
    class WebDriverException : Exception
    {
        public string Error { get; }

        public WebDriverException(string error, string message) : base(message)
        {
            Error = error;
        }
    }

    class NativeUIServer
    {
        // The name of the element reference in the W3C protocol.
        const string ElementKey = "element-6066-11e4-a52e-4f735466cecf";

        readonly int port;
        readonly int targetPid;
        readonly UiaTree tree;
        readonly HashSet<string> sessionIds = new();
        readonly Dictionary<string, AutomationElement> elements = new();

        public NativeUIServer(int port, int targetPid)
        {
            this.port = port;
            this.targetPid = targetPid;
            tree = new UiaTree(targetPid);
        }

        public void Run()
        {
            var listener = new TcpListener(IPAddress.Loopback, port);
            listener.Start();
            while (true)
            {
                using TcpClient client = listener.AcceptTcpClient();
                try
                {
                    Serve(client.GetStream());
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine($"coda-driver: {e}");
                }
            }
        }

        record Request(string Method, string Path, byte[] Body);

        static Request? ReadRequest(NetworkStream stream)
        {
            var buffer = new MemoryStream();
            var chunk = new byte[65536];
            int headerEnd = -1;
            while (headerEnd < 0)
            {
                int count = stream.Read(chunk, 0, chunk.Length);
                if (count == 0)
                    return null;
                buffer.Write(chunk, 0, count);
                headerEnd = new ReadOnlySpan<byte>(buffer.GetBuffer(), 0, (int)buffer.Length)
                                .IndexOf("\r\n\r\n"u8);
            }

            byte[] data = buffer.ToArray();
            string header = Encoding.UTF8.GetString(data, 0, headerEnd);
            string[] lines = header.Split("\r\n");
            string[] requestLine = lines[0].Split(' ');
            if (requestLine.Length < 2)
                return null;

            int contentLength = 0;
            foreach (string line in lines)
            {
                if (line.StartsWith("content-length:", StringComparison.OrdinalIgnoreCase))
                    int.TryParse(line.Substring("content-length:".Length).Trim(),
                                 out contentLength);
            }

            var body = new MemoryStream();
            body.Write(data, headerEnd + 4, data.Length - headerEnd - 4);
            while (body.Length < contentLength)
            {
                int count = stream.Read(chunk, 0, chunk.Length);
                if (count == 0)
                    break;
                body.Write(chunk, 0, count);
            }

            return new Request(requestLine[0], requestLine[1], body.ToArray());
        }

        void Serve(NetworkStream stream)
        {
            Request? request = ReadRequest(stream);
            if (request == null)
                return;

            string status = "200 OK";
            object? value;
            try
            {
                value = Route(request);
            }
            catch (WebDriverException e)
            {
                status = e.Error == "invalid session id" ? "404 Not Found"
                                                         : "500 Internal Server Error";
                value = new Dictionary<string, object>
                {
                    ["error"] = e.Error,
                    ["message"] = e.Message,
                    ["stacktrace"] = "",
                };
            }
            catch (ElementNotAvailableException e)
            {
                status = "404 Not Found";
                value = new Dictionary<string, object>
                {
                    ["error"] = "stale element reference",
                    ["message"] = e.Message,
                    ["stacktrace"] = "",
                };
            }

            byte[] body = JsonSerializer.SerializeToUtf8Bytes(
                new Dictionary<string, object?> { ["value"] = value });
            byte[] head = Encoding.ASCII.GetBytes(
                $"HTTP/1.1 {status}\r\nContent-Type: application/json; charset=utf-8\r\n"
                + $"Content-Length: {body.Length}\r\nConnection: close\r\n\r\n");
            stream.Write(head);
            stream.Write(body);
        }

        object? Route(Request request)
        {
            string[] segments = request.Path.Split('/', StringSplitOptions.RemoveEmptyEntries);

            if (request.Method == "GET" && request.Path == "/status")
            {
                return new Dictionary<string, object>
                {
                    ["ready"] = true,
                    ["message"] = "coda-driver-native",
                    ["targetPid"] = targetPid,
                    ["driverPid"] = Environment.ProcessId,
                };
            }

            if (request.Method == "POST" && segments.Length == 1 && segments[0] == "session")
            {
                string id = Guid.NewGuid().ToString();
                sessionIds.Add(id);
                return new Dictionary<string, object>
                {
                    ["sessionId"] = id,
                    ["capabilities"] = new Dictionary<string, object>(),
                };
            }

            if (request.Method == "DELETE" && segments.Length == 2 && segments[0] == "session")
            {
                sessionIds.Remove(segments[1]);
                elements.Clear();
                return null;
            }

            if (segments.Length < 3 || segments[0] != "session")
                throw NotImplemented(request);
            if (!sessionIds.Contains(segments[1]))
                throw new WebDriverException("invalid session id",
                                             $"No active session with id '{segments[1]}'");

            string[] sub = segments[2..];

            if (request.Method == "POST" && sub.Length == 1 && sub[0] == "element")
            {
                var predicate = ParsePredicate(request.Body);
                UiaTree.Found found = tree.Find(predicate, firstOnly: true);
                if (found.Elements.Count == 0)
                    throw new WebDriverException("no such element", NotFoundMessage(predicate,
                                                                                    found));
                return Reference(found.Elements[0]);
            }

            if (request.Method == "POST" && sub.Length == 1 && sub[0] == "elements")
            {
                var predicate = ParsePredicate(request.Body);
                UiaTree.Found found = tree.Find(predicate, firstOnly: false);
                var references = new List<object>();
                foreach (AutomationElement element in found.Elements)
                    references.Add(Reference(element));
                return references;
            }

            if (request.Method == "GET" && sub.Length == 1 && sub[0] == "source")
                return tree.PageSource();

            if (sub.Length == 3 && sub[0] == "element")
            {
                AutomationElement element = Lookup(sub[1]);
                switch (request.Method, sub[2])
                {
                    case ("POST", "click"):
                        UiaTree.Click(element);
                        return null;
                    case ("POST", "value"):
                        UiaTree.SetValue(element, TextOf(request.Body));
                        return null;
                    case ("POST", "clear"):
                        UiaTree.Clear(element);
                        return null;
                    case ("GET", "displayed"):
                        return !element.Current.IsOffscreen;
                }
            }

            throw NotImplemented(request);
        }

        static WebDriverException NotImplemented(Request request)
        {
            return new WebDriverException("unknown command",
                                          $"{request.Method} {request.Path} not implemented");
        }

        Dictionary<string, string> Reference(AutomationElement element)
        {
            string id = Guid.NewGuid().ToString();
            elements[id] = element;
            return new Dictionary<string, string> { [ElementKey] = id };
        }

        AutomationElement Lookup(string id)
        {
            if (!elements.TryGetValue(id, out var element))
                throw new WebDriverException("stale element reference",
                                             $"Element '{id}' no longer exists");
            return element;
        }

        static string NotFoundMessage(UiaTree.Predicate predicate, UiaTree.Found found)
        {
            string message = $"No element with {predicate.Attribute}=\"{predicate.Value}\"";
            if (found.TimedOut)
                message += $" in the {UiaTree.WalkSeconds} seconds that a walk of the tree may"
                    + " take; the tree is larger than that, so the element may exist further on";
            return message;
        }

        static readonly Regex XPathPattern = new(@"^//\*\[@([\w-]+)\s*=\s*""([^""]*)""\]$");

        static UiaTree.Predicate ParsePredicate(byte[] body)
        {
            using JsonDocument json = ParseJson(body);
            if (!json.RootElement.TryGetProperty("using", out JsonElement usingElement)
                || !json.RootElement.TryGetProperty("value", out JsonElement valueElement))
                throw new WebDriverException("invalid argument", "Missing 'using' or 'value'");

            string? strategy = usingElement.GetString();
            string? selector = valueElement.GetString();
            if (strategy != "xpath")
                throw new WebDriverException("invalid argument",
                                             $"Only 'xpath' is supported (got '{strategy}')");

            Match match = XPathPattern.Match(selector ?? "");
            if (!match.Success)
                throw new WebDriverException("invalid argument", $"Unsupported XPath: {selector}");

            return new UiaTree.Predicate(match.Groups[1].Value, match.Groups[2].Value);
        }

        // The text to type, either as "text" or as the array of characters in "value".
        static string TextOf(byte[] body)
        {
            using JsonDocument json = ParseJson(body);
            if (json.RootElement.TryGetProperty("text", out JsonElement text)
                && text.ValueKind == JsonValueKind.String)
                return text.GetString() ?? "";

            if (json.RootElement.TryGetProperty("value", out JsonElement value)
                && value.ValueKind == JsonValueKind.Array)
            {
                var result = new StringBuilder();
                foreach (JsonElement item in value.EnumerateArray())
                    result.Append(item.GetString());
                return result.ToString();
            }

            throw new WebDriverException("invalid argument", "Missing 'text' or 'value' in body");
        }

        static JsonDocument ParseJson(byte[] body)
        {
            try
            {
                return JsonDocument.Parse(body);
            }
            catch (JsonException e)
            {
                throw new WebDriverException("invalid argument", $"Bad JSON in body: {e.Message}");
            }
        }
    }
}
