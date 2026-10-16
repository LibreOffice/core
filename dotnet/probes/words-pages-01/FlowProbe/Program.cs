// Dumps every placed line of a word-processing document: which paragraph it came from, the top of
// its box, its height, the space above it, the face it resolved to and its text.
//
// WHY a dump rather than a page count. A one-page shortfall is a sum of per-line deficits, and a PDF
// shows only the lines that carry ink — an empty paragraph is invisible to `pdftotext -bbox` on both
// sides, so the arithmetic that says *which* paragraph is short cannot be done from the output. This
// prints the box, so a missing 41.4 pt can be attributed to the paragraph that owns it.
using XRay.Office;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;

string path = args[0];
int lastPage = args.Length > 1 ? int.Parse(args[1], System.Globalization.CultureInfo.InvariantCulture) : 2;

using IDocument doc = OfficeDocument.Open(path);
var pages = (WordProcessingPages)((IPaginatedDocument)doc).Layout();

foreach (LaidOutPage page in pages.Pages)
{
    if (page.Index >= lastPage) break;
    Console.WriteLine($"--- page {page.Index + 1}  body top={page.BodyArea.Top.Points:F2} "
        + $"height={page.BodyArea.Height.Points:F2}  lines={page.Lines.Count}");
    foreach (PlacedLine line in page.Lines)
    {
        var blocks = page.Blocks ?? pages.Blocks;
        string text = pages.TextOf(line);
        string face = "-";
        double size = 0;
        if (line.ParagraphIndex >= 0 && line.ParagraphIndex < blocks.Count
            && blocks[line.ParagraphIndex] is PageParagraph p)
        {
            face = p.Font?.FamilyName ?? p.Face.FamilyName;
            size = p.EmSize.Points;
        }

        string runs = "-";
        if (line.ParagraphIndex >= 0 && line.ParagraphIndex < blocks.Count
            && blocks[line.ParagraphIndex] is PageParagraph q)
        {
            runs = string.Join(",", q.Runs.Select(
                r => $"{r.Start}+{r.Length}@{r.EmSize.Points:F1}"));
            if (runs.Length == 0) runs = "(none)";
        }

        Console.WriteLine(
            $"  p{line.ParagraphIndex,4}.{line.LineIndex} top={line.Top.Points,8:F2} "
            + $"h={line.Box.Height.Points,7:F2} up={line.UpperSpace.Points,6:F2} "
            + $"sa={line.Box.SpaceAbove.Points,6:F2} {size,5:F1}pt {face,-22} "
            + $"runs=[{runs}] len={text.Length} | "
            + (text.Length > 40 ? text[..40] : text));
    }
}
