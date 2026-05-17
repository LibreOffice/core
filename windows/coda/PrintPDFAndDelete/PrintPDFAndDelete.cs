// -*- tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// A program that prints the PDF file given as the single command-line argument. The argument is the
// file: URI for the file.
//
// Written as a Windows Forms application because I found simple sample code for how to print a PDF
// document from C#, and a Windows Forms application apparently is the simplest way to have a
// message pump, which the code in question seems to require.
//
// It has a single Form that is actually hidden as soon as possble.
//
// The actual printing is done from an IMessageFilter... i couldn't quickly come up with other way
// to do it. It can't be done before the message pump is up and running.
//
// Once the printing is done the program exits.

using System.Drawing.Printing;
using System.IO;
using System.Windows;
using Application = System.Windows.Forms.Application;

namespace PrintPDFAndDelete
{
    internal static class PrintPDFAndDelete
    {
        // Assigned in Main before any read; null-forgiving silences CS8618.
        private static string[] _args = null!;
        private static Form _dummy = null!;

        [STAThread]
        static void Main(string[] args)
        {
            _args = args;
            Application.AddMessageFilter(new StupidMessageFilter());
            _dummy = new Form();
            // We don't want to see any flashing window border
            _dummy.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            _dummy.ShowInTaskbar = false;
            Application.Run(_dummy);
        }

        private class StupidMessageFilter : IMessageFilter
        {
            static bool _firstTime = true;

            public bool PreFilterMessage(ref Message m)
            {
                if (_firstTime)
                {
                    _firstTime = false;
                    _dummy.Hide();
                    PrintPdfDocument(_args[0]);
                }
                return false;
            }
        }

        static private async Task<List<System.Drawing.Image>> PdfToImage(string path)
        {
            // Use standard .NET file I/O instead of WinRT GetFileFromPathAsync
            // to avoid file access broker "access denied" errors on temp files
            using var fileStream = File.OpenRead(path);
            using var randomAccessStream = fileStream.AsRandomAccessStream();
            Windows.Data.Pdf.PdfDocument pdfDocument = await Windows.Data.Pdf.PdfDocument.LoadFromStreamAsync(randomAccessStream);

            uint index = 0;
            List<System.Drawing.Image> images = new List<System.Drawing.Image>();

            while (index < pdfDocument.PageCount)
            {
                using (Windows.Data.Pdf.PdfPage pdfPage = pdfDocument.GetPage(index))
                {
                    using (Windows.Storage.Streams.InMemoryRandomAccessStream memStream = new Windows.Storage.Streams.InMemoryRandomAccessStream())
                    {
                        Windows.Data.Pdf.PdfPageRenderOptions pdfPageRenderOptions = new Windows.Data.Pdf.PdfPageRenderOptions();
                        await pdfPage.RenderToStreamAsync(memStream);
                        System.Drawing.Image image = System.Drawing.Image.FromStream(memStream.AsStream());
                        images.Add(image);
                    }
                }
                index++;
            }

            return images;
        }

       static private async void PrintPdfDocument(string uri)
        {
            try
            {
                var path = new Uri(uri).LocalPath;
                var images = await PdfToImage(path);
                int index = 0;

                PrintDocument printDocument = new PrintDocument();

                printDocument.PrintPage += (sender, e) =>
                {
                    System.Drawing.Image image = images[index];

                    // Calculate the scaling factor to fit the image within the page size
                    float X = (float)e.PageSettings.PaperSize.Width / image.Width;
                    float Y = (float)e.PageSettings.PaperSize.Height / image.Height;
                    float scaleFactor = Math.Min(X, Y);

                    // Draw the image on the page
                    e.Graphics!.DrawImage(image, 0, 0, image.Width * scaleFactor, image.Height * scaleFactor);

                    index++;
                    if (index < images.Count)
                    {
                        e.HasMorePages = true;
                        image.Dispose();
                        return;
                    }

                    e.HasMorePages = false;
                    image.Dispose();
                };

                PrintDialog printDialog = new PrintDialog();
                printDialog.AllowCurrentPage = true;
                printDialog.AllowSomePages = true;
                printDialog.Document = printDocument;
                if (printDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    printDocument.Print();
                }

                File.Delete(path);
                // This is called by CODA to print a PDF that is the only file in a temporary
                // directory. But if you when testing or otherwise happen to call it with a file
                // that is not the only one in its directory, Directory.Delete() will not do
                // anything, you won't accidentally lose other files.
                Directory.Delete(Path.GetDirectoryName(path)!);
                Application.Exit();
            }
            catch (System.Exception ex)
            {
                System.Windows.MessageBox.Show("Error : " + ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}
