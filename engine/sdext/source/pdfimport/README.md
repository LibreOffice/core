# PDF import

## Introduction

Opening a PDF file produces a Draw document with one rendered image per
page. The rendering is done by Pdfium via the vcl PDF filter, and the
Draw document is built by `SdPdfFilter`
(sd/source/filter/pdf/sdpdffilter.cxx).

A rendered page can be turned into editable drawing-level shapes (text,
paths, images) on demand: the Break action (`DoImportMarkedMtf` in
svx/source/svdraw/svdedtv2.cxx) or the `DecomposePDF` export option both
run `ImpSdrPdfImport` (svx/source/svdraw/svdpdf.cxx), which uses Pdfium's
page model to emit the shapes. So PDF text is recoverable via Pdfium, just
not on the initial open.

This directory provides the PDF type detection and the hybrid PDF
handling described below.

## Hybrid documents

A PDF can embed the original document it was generated from (a "hybrid"
PDF+ODF). When such a file is opened, the embedded original is extracted
and loaded instead of the rendered PDF, so the result is fully editable.

## Structure

- *filterdet.cxx* - `PDFDetector`. Detects PDF input and, for a hybrid
  file, finds the embedded original. It reads an embedded-file attachment
  via Pdfium (vcl/filter/PDFiumLibrary), or the older trailer-based
  AdditionalStream via *pdfparse/*.

- *pdfiadaptor.cxx* - `PDFIHybridAdaptor`. Extracts the embedded original
  substream identified during detection and hands it to the matching
  document importer (OwnSubFilter).

- *pdfparse/* - a small PDF tokenizer used by the detector and by the
  *pdfunzip* debug tool (test/pdfunzip.cxx).

## Bug handling

- Please tag bugs with *filter:pdf* in component *filters and storage*.

- [qpdf](https://github.com/qpdf/qpdf) is useful for editing raw PDF
  files to cut down the number of primitives when reducing a problem.
