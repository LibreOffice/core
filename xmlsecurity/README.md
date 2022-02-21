# Stuff for Document Signing

This code provides dialogs, and infrastructure wrapping `libxmlsec` and
`gpgme` that implements document signing.

For signing a document, a personal key pair is used, which consists of a
private key and a public key, which is added to the document in addition
to the digital signature of the document, when signing it.

The document signing can be done both for the source ODF/OOXML files and
the exported PDF files. It is also possible to sign existing PDF files.

To test the signed PDFs, one can use the `pdfverify` in this way:

    ./bin/run pdfverify $PWD/xmlsecurity/qa/unit/pdfsigning/data/2good.pdf

The file parameter should be an absolute path.

This is the output of `pdfverify` for `2good.pdf`:

```
verifying signatures
found 2 signatures
signature #0: digest match? 1
signature #0: partial? 0
signature #1: digest match? 1
signature #1: partial? 0
```

## References
* [Adobe: Digital Signatures in a PDF](https://www.adobe.com/devnet-docs/etk_deprecated/tools/DigSig/Acrobat\_DigitalSignatures_in_PDF.pdf)
* [Adobe: Acrobat DC Digital Signatures - Supported Standards](https://www.adobe.com/devnet-docs/acrobatetk/tools/DigSigDC/standards.html)
