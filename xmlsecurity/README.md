# Document Signing

## Introduction

This code provides dialogs, and infrastructure wrapping `libxmlsec` and
`gpgme` that implements document signing.

For signing a document, a personal key pair is used, which consists of a
private key and a public key, which is added to the document in addition
to the digital signature of the document, when signing it.

The document signing can be done both for the source ODF/OOXML files and
the exported PDF files. It is also possible to sign existing PDF files.

## Module Contents

 * `doc`: OpenDocument workflow legacy information with some illustrations to have an idea of the workflow, for starters check `doc/OpenDocumentSignatures-Workflow.odt`.
 * `inc`: Headers to a subset of source files inside the module, parts like `source/framework` have headers inside the folder.
 * `qa`: Unit tests for signing and shell scripts for certificates creation for testing.
 * `test_docs`: Documents & certificates used for testing.
 * `source`: More on that below.
 * `uiconfig`: User interface configuration for different dialogs, it is recommended to navigate from relevant source file to the .ui file linked in the class which will be under `uiconfig/ui`.
 * `util`: UNO passive registration config for GPG/ NSS.

## Source Primary Contents

 * `component`: Main implementation of `DocumentDigitalSignatures` where the interaction with security environment and certificates occur.
 * `dialogs`: Certificate & Signatures management dialogs.
   * `certificatechooser`: Dialog that allows you to find and choose certificates or signatures for encryption.
   * `certificateviewer`: More detailed information about each certificate.
   * `digitalsignaturesdialog`: Main window for signatures of the documents and the start point of signing document.
 * `framework`: Various elements for verifying signatures and running security engine.
 * `gpg`: The implementation of encrypting with GPG and security environment initialization.
 * `helper`: Some helper classes that include signatures manager and the helpers for PDF signing, UriBinding, and XML signatures. It also include helper tools for XSecurityEnvironment.
 * `xmlsec`: XML, NSS, MSCrypt encryption/ signing tools, more on the low-level side of actual implementation of algorithms.


## PDF Testing

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
