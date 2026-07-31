using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;

namespace Paperless.Fidelity.Tests;

public sealed class TempDump
{
    [Fact]
    public void Dump()
    {
        string path = Corpus.Require("tabbed.fodt");
        RecordingDrawingSink sink = new();
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);

        List<string> dump = [];
        foreach (PageParagraph p in pages.Paragraphs)
        {
            dump.Add($"para stops={p.Format.TabStops.Count} interval={p.Format.DefaultTabInterval.Twips} "
                     + $"[{p.Text.Replace('\t', '|')}]");
        }
        foreach (DrawnPage page in sink.Pages)
        {
            foreach (DrawnWord w in DrawnWords.On(page))
            {
                dump.Add($"  {w.Baseline,8:F2} {w.Left,9:F3}..{w.Right,9:F3} \"{w.Text}\"");
            }
        }

        File.WriteAllLines(
            Path.Combine("/tmp/claude-0/-home-user-libreoffice-core",
                "97fb9a5c-d3af-5181-8a45-3e02e688a651", "scratchpad", "dump", "tabs.txt"),
            dump);
    }
}
