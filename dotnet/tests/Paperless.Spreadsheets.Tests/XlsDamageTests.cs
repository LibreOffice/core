using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests what the reader does with workbooks that are wrong.
/// </summary>
/// <remarks>
/// A thirty-year-old format written by dozens of applications produces files that break their
/// own rules constantly, and the reader's contract is a diagnostic and partial content rather
/// than an exception. The damage here is inflicted on a real workbook rather than invented,
/// so the container around it stays valid and only the BIFF layer is broken — which is the
/// shape most real damage takes.
/// </remarks>
public class XlsDamageTests
{
    private static byte[] Workbook() => File.ReadAllBytes(Corpus.Require("xls-features.xls"));

    private static IDocument Open(byte[] bytes)
        => new SpreadsheetReader().Read(DocumentSource.FromBytes(bytes, "damaged.xls"));

    [Fact]
    public void ASheetDirectoryPointingAtNothingFallsBackToScanningForTheSubstream()
    {
        byte[] bytes = Workbook();

        // Every BOUNDSHEET record starts with the absolute offset of its sheet's BOF, written
        // after the fact by the exporter and so the field most likely to be wrong. LibreOffice
        // carries the same fallback for the same reason (read.cxx:52-66, i#115255).
        int patched = 0;
        for (int i = 0; i + 10 < bytes.Length; i++)
        {
            if (bytes[i] != 0x85 || bytes[i + 1] != 0x00) continue;

            // A plausible BOUNDSHEET: a short record whose visibility field is one of the
            // three legal values.
            int length = bytes[i + 2] | (bytes[i + 3] << 8);
            if (length is < 8 or > 64) continue;
            if (bytes[i + 9] > 2) continue;

            bytes[i + 4] = 0xFF;
            bytes[i + 5] = 0xFF;
            bytes[i + 6] = 0x00;
            bytes[i + 7] = 0x00;
            patched++;
        }

        patched.ShouldBeGreaterThan(0);

        using IDocument document = Open(bytes);
        document.Diagnostics.ShouldContain(d => d.Code == "PL2326");
        document.Content.Children.OfType<ContentSection>().Count().ShouldBe(4);
    }

    [Fact]
    public void ATruncatedWorkbookIsRefusedByTheContainerRatherThanMisread()
    {
        byte[] whole = Workbook();
        byte[] cut = whole[..(whole.Length * 3 / 4)];

        // A compound file keeps its directory at the end, so cutting a file this size takes
        // the map of where everything is with it. That is the container's business rather
        // than the workbook reader's, and it is right that it fails loudly: half a directory
        // means the streams cannot be located at all, which is not the same as a workbook
        // with a damaged record in it.
        using DocumentSource source = DocumentSource.FromBytes(cut, "truncated.xls");
        Should.Throw<Exception>(() => MsBinary.XlsReader.Read(source, DocumentFormat.Xls))
              .ShouldBeAssignableTo<Exception>();
    }

    [Fact]
    public void AFileThatIsNotAWorkbookAtAllIsRejectedClearly()
    {
        // An OLE2 compound file that holds a Word document, offered as a workbook.
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require("prose-doc.doc"));

        MalformedDocumentException rejected = Should.Throw<MalformedDocumentException>(
            () => MsBinary.XlsReader.Read(source, DocumentFormat.Xls));
        rejected.Message.ShouldContain("Workbook");
    }

    [Fact]
    public void ARecordWhoseLengthLiesDerailsTheWalkWithoutThrowing()
    {
        byte[] bytes = Workbook();

        // Halve the declared length of every LABELSST record. A BIFF stream has no
        // synchronisation marker of any kind — the length field is the only thing that says
        // where the next record starts — so a wrong length cannot be recovered from, and the
        // rest of the substream is read as whatever the misaligned bytes happen to spell.
        // LibreOffice has the same property, and the point of this test is that the failure
        // is a wrong reading rather than a crash.
        for (int i = 0; i + 4 < bytes.Length; i++)
        {
            if (bytes[i] == 0xFD && bytes[i + 1] == 0x00 && bytes[i + 2] == 0x0A && bytes[i + 3] == 0x00)
                bytes[i + 2] = 0x05;
        }

        using IDocument document = Open(bytes);

        // It read something, said the file was damaged, and did not throw.
        document.Content.Children.ShouldNotBeEmpty();
        document.Diagnostics.ShouldContain(d => d.Severity >= DiagnosticSeverity.Warning);
    }
}
