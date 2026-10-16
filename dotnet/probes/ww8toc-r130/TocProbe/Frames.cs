// The frames a word-processing document lays out, page by page: what a rendering shows as a
// picture, with the anchor and the inline ascent that decide where it goes.
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing;
using XRay.Office.WordProcessing.Layout;

internal static class Frames
{
    internal static void Dump(string path, int from, int to)
    {
        using IDocument document = new WordProcessingReader().Read(DocumentSource.FromFile(path));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        for (int i = from - 1; i < Math.Min(to, pages.Pages.Count); i++)
        {
            LaidOutPage page = pages.Pages[i];
            Console.WriteLine($"page {i + 1}: {page.Frames.Count} frame(s), {page.Lines.Count} line(s)");
            foreach (PlacedFrame placed in page.Frames)
            {
                Console.WriteLine(
                    $"  {placed.Frame.Anchor} wrap={placed.Frame.Wrap} "
                    + $"size={placed.Frame.Size.Width.Points:F2}x{placed.Frame.Size.Height.Points:F2} "
                    + $"ascent={placed.Frame.InlineAscent?.Points.ToString("F2") ?? "-"} "
                    + $"area=({placed.Area.X.Points:F2},{placed.Area.Y.Points:F2}) "
                    + $"{placed.Area.Width.Points:F2}x{placed.Area.Height.Points:F2}");
            }

            if (page.Header is { } head)
            {
                Console.WriteLine($"  header area=({head.Area.X.Points:F2},{head.Area.Y.Points:F2}) "
                    + $"{head.Area.Width.Points:F2}x{head.Area.Height.Points:F2} lines={head.Lines.Count}");
                foreach (PlacedLine l in head.Lines)
                {
                    Console.WriteLine($"    hline p{l.ParagraphIndex}#{l.LineIndex} top={l.Top.Points:F2} "
                        + $"h={l.Box.Height.Points:F2} base={l.Box.Baseline.Points:F2} w={l.Box.Width.Points:F2}");
                }

            }

            foreach (PageParagraph one in pages.Paragraphs.Where(q => q.Text.Contains('\u0001')).Take(4))
            {
                Console.WriteLine($"  para {System.Text.Json.JsonSerializer.Serialize(one.Text)}");
            }

            foreach (PlacedLine line in page.Lines.Take(4))
            {
                Console.WriteLine(
                    $"  line p{line.ParagraphIndex}#{line.LineIndex} top={line.Top.Points:F2} "
                    + $"h={line.Box.Height.Points:F2} base={line.Box.Baseline.Points:F2} "
                    + $"w={line.Box.Width.Points:F2}");
            }
        }
    }
}
