using System.Globalization;
using System.Text;

namespace XRay.Office.Rendering.Pdf;

/// <summary>
/// What lets a reader move around a PDF: link annotations, named destinations, the outline, and the
/// labels a page is known by.
/// </summary>
/// <remarks>
/// <para>
/// All of it is written once the last page has been drawn, because a link on page 3 may name a place on
/// page 40 and an outline entry names a page object whose number only exists after that page has been
/// written. The sink collects the pieces as the pages are drawn (<see cref="PdfLink"/>,
/// <see cref="PdfDestination"/>, <see cref="PdfOutlineEntry"/>) and this turns them into objects.
/// </para>
/// <para>
/// An internal link whose destination was never declared is dropped rather than written dangling: a
/// <c>/Link</c> with nothing to go to is a rectangle that does nothing, and a viewer would still draw a
/// pointer over it.
/// </para>
/// </remarks>
internal static class PdfNavigation
{
    /// <summary>
    /// Writes the link annotations of every page and returns each page's <c>/Annots</c> entry.
    /// </summary>
    /// <param name="writer">Where the objects go.</param>
    /// <param name="pageIds">The object number of each page.</param>
    /// <param name="links">Each page's links.</param>
    /// <param name="destinations">The named places.</param>
    public static string[] WriteAnnotations(
        PdfDocumentWriter writer,
        IReadOnlyList<int> pageIds,
        IReadOnlyList<IReadOnlyList<PdfLink>> links,
        IReadOnlyDictionary<string, PdfDestination> destinations)
    {
        string[] entries = new string[pageIds.Count];

        for (int page = 0; page < pageIds.Count; page++)
        {
            StringBuilder annots = new();

            foreach (PdfLink link in links[page])
            {
                string? action = Action(link, pageIds, destinations);
                if (action is null) continue;

                string border = "/Border[0 0 0]";
                int id = writer.Add(
                    $"<</Type/Annot/Subtype/Link/Rect[{N(link.Left)} {N(link.Bottom)} {N(link.Right)} {N(link.Top)}]"
                    + $"{border}/F 4{action}>>");
                annots.Append(CultureInfo.InvariantCulture, $"{id} 0 R ");
            }

            entries[page] = annots.Length == 0 ? string.Empty : $"/Annots[{annots.ToString().TrimEnd()}]";
        }

        return entries;
    }

    /// <summary>A named place as a destination array, or null when its page is not in the file.</summary>
    public static string? Destination(PdfDestination at, IReadOnlyList<int> pageIds)
    {
        if (at.Page < 0 || at.Page >= pageIds.Count) return null;

        return string.Create(
            CultureInfo.InvariantCulture,
            $"[{pageIds[at.Page]} 0 R/XYZ {N(at.X)} {N(at.Y)} null]");
    }

    private static string? Action(
        PdfLink link, IReadOnlyList<int> pageIds, IReadOnlyDictionary<string, PdfDestination> destinations)
    {
        if (link.Uri is { } uri) return $"/A<</S/URI/URI{PdfSyntax.LiteralString(Ascii(uri))}>>";

        if (link.Destination is { } name
            && destinations.TryGetValue(name, out PdfDestination at)
            && Destination(at, pageIds) is { } array)
        {
            return $"/Dest{array}";
        }

        return null;
    }

    /// <summary>
    /// An address as the 7-bit text a <c>/URI</c> action requires: anything outside ASCII is
    /// percent-encoded as UTF-8, and a character that cannot appear in an address at all is dropped.
    /// </summary>
    public static string Ascii(string uri)
    {
        StringBuilder text = new(uri.Length);
        Span<byte> bytes = stackalloc byte[4];
        foreach (Rune rune in uri.EnumerateRunes())
        {
            if (rune.Value is > ' ' and < 0x7F)
            {
                text.Append((char)rune.Value);
            }
            else if (rune.Value == ' ')
            {
                text.Append("%20");
            }
            else if (rune.Value >= 0x80 && rune.Value != 0xFFFD)
            {
                int count = rune.EncodeToUtf8(bytes);
                for (int i = 0; i < count; i++) text.Append('%').Append(bytes[i].ToString("X2", CultureInfo.InvariantCulture));
            }
        }

        return text.ToString();
    }

    /// <summary>
    /// Writes the outline and returns the object number of its root, or 0 when there is nothing to write.
    /// </summary>
    public static int WriteOutline(
        PdfDocumentWriter writer, IReadOnlyList<PdfOutlineEntry> entries, IReadOnlyList<int> pageIds)
    {
        List<Node> roots = [];
        Stack<Node> open = new();

        foreach (PdfOutlineEntry entry in entries)
        {
            if (Destination(entry.Target, pageIds) is not { } dest) continue;

            Node node = new(entry.Level, entry.Title, dest);
            while (open.Count > 0 && open.Peek().Level >= entry.Level) open.Pop();

            if (open.Count == 0) roots.Add(node);
            else open.Peek().Children.Add(node);
            open.Push(node);
        }

        if (roots.Count == 0) return 0;

        int root = writer.Reserve();
        Assign(writer, roots);

        writer.Set(
            root,
            string.Create(
                CultureInfo.InvariantCulture,
                $"<</Type/Outlines/First {roots[0].Id} 0 R/Last {roots[^1].Id} 0 R/Count {Visible(roots)}>>"));
        Write(writer, roots, root);

        return root;
    }

    private static void Assign(PdfDocumentWriter writer, List<Node> nodes)
    {
        foreach (Node node in nodes)
        {
            node.Id = writer.Reserve();
            Assign(writer, node.Children);
        }
    }

    private static int Visible(List<Node> nodes)
    {
        int count = 0;
        foreach (Node node in nodes) count += 1 + Visible(node.Children);
        return count;
    }

    private static void Write(PdfDocumentWriter writer, List<Node> siblings, int parent)
    {
        for (int i = 0; i < siblings.Count; i++)
        {
            Node node = siblings[i];

            StringBuilder body = new();
            body.Append("<</Title").Append(PdfSyntax.LiteralString(node.Title));
            body.Append(CultureInfo.InvariantCulture, $"/Parent {parent} 0 R");
            if (i > 0) body.Append(CultureInfo.InvariantCulture, $"/Prev {siblings[i - 1].Id} 0 R");
            if (i + 1 < siblings.Count) body.Append(CultureInfo.InvariantCulture, $"/Next {siblings[i + 1].Id} 0 R");

            if (node.Children.Count > 0)
            {
                body.Append(CultureInfo.InvariantCulture,
                    $"/First {node.Children[0].Id} 0 R/Last {node.Children[^1].Id} 0 R/Count {Visible(node.Children)}");
            }

            body.Append("/Dest").Append(node.Destination).Append(">>");
            writer.Set(node.Id, body.ToString());

            Write(writer, node.Children, node.Id);
        }
    }

    /// <summary>
    /// Writes the name tree of destinations and returns its object number, or 0 when there are none.
    /// </summary>
    public static int WriteNames(
        PdfDocumentWriter writer,
        IReadOnlyDictionary<string, PdfDestination> destinations,
        IReadOnlyList<int> pageIds)
    {
        List<string> names = [.. destinations.Keys];
        names.Sort(StringComparer.Ordinal);

        StringBuilder pairs = new();
        foreach (string name in names)
        {
            if (Destination(destinations[name], pageIds) is not { } array) continue;
            pairs.Append(PdfSyntax.LiteralString(name)).Append(array);
        }

        return pairs.Length == 0 ? 0 : writer.Add($"<</Dests<</Names[{pairs}]>>>>");
    }

    /// <summary>
    /// The <c>/PageLabels</c> number tree for pages whose labels are plain numbers, or an empty string
    /// when the labels are anything else (a sheet's name, a slide's title) or already the page indexes.
    /// </summary>
    /// <remarks>
    /// One range is written per run of consecutive numbers, so a document that restarts its numbering
    /// after a title page produces two. Labels that are not decimal numbers have no place in the number
    /// tree — it can express a style and a prefix, but the layout knows only the printed text — and a file
    /// that states labels for some pages and not others would misnumber the rest, so the whole tree is
    /// left out.
    /// </remarks>
    public static string PageLabels(IReadOnlyList<string?> labels)
    {
        if (labels.Count == 0) return string.Empty;

        int[] numbers = new int[labels.Count];
        for (int i = 0; i < labels.Count; i++)
        {
            if (!int.TryParse(labels[i], NumberStyles.None, CultureInfo.InvariantCulture, out numbers[i]) || numbers[i] < 1)
                return string.Empty;
        }

        StringBuilder nums = new();
        bool differs = false;
        for (int i = 0; i < numbers.Length; i++)
        {
            if (i > 0 && numbers[i] == numbers[i - 1] + 1) continue;

            if (numbers[i] != i + 1) differs = true;
            nums.Append(CultureInfo.InvariantCulture, $"{i}<</S/D/St {numbers[i]}>>");
        }

        // Numbers that are simply 1, 2, 3 describe PDF's own default and are not worth the object.
        return differs ? $"/PageLabels<</Nums[{nums}]>>" : string.Empty;
    }

    private static string N(double value) => PdfSyntax.Number(value);

    private sealed class Node(int level, string title, string destination)
    {
        public int Level { get; } = level;
        public string Title { get; } = title;
        public string Destination { get; } = destination;
        public int Id { get; set; }
        public List<Node> Children { get; } = [];
    }
}
