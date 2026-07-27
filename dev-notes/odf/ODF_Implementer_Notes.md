
# ODF Implementer Notes

This page concerns **ODF-specific Implementer Notes** for Collabora
Online and Collabora Office developers.

## ODF Extensions

### ODF Version Choice

In the code, look for `ODFVER_*` enum values - currently, there is
`ODFVER_UNKNOWN`, `ODFVER_010`, `ODFVER_011`, `ODFVER_012`,
`ODFVER_012_EXT_COMPAT`, `ODFVER_012_EXTENDED`, `ODFVER_013` and
`ODFVER_LATEST` (this means ODF 1.3 Extended at the moment).

`ODFSVER_012_EXT_COMPAT` should be used to fallback to old, incorrect
output - we e.g. use that to write svg:d path statements that are
interpreted wrongly by buggy legacy ODF consumers.

The difference between `ODFSaneDefaultVersion` and `ODFDefaultVersion` enums
is that `ODFSaneDefaultVersion` provides enums that can be used in exact
comparisons and the "plain" ODF version is smaller than the same version
with extensions allowed and if extensions are allowed the
`ODFSVER_EXTENDED` bit is set and
`SvtSaveOptions::GetODFSaneDefaultVersion()` returns always a specific ODF
version, whereas `ODFDefaultVersion::ODFVER_LATEST` is a running target
meaning always the latest and greatest that you don't know which it
actually is.

#### Manually exported elements and attributes

If you implemented a feature that needs to extend ODF - **please** use
the following pattern:

     // export only if ODF extensions are enabled
     if (rExport.getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
     {
      ...
     }

Obtained through `SvXMLExport::getSaneDefaultVersion()` (don't call
`SvtSaveOptions::GetODFSaneDefaultVersion()` directly because there
could be an override in `SvXMLExport`), this is how features that are
already part of a Committee Specification Draft at OASIS can be
exported:

    SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
    if (eVersion >= SvtSaveOptions::ODFSVER_013)
    {
        ... write ODF 1.3 namespace etc. ...
    }
    else if ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0)
    {
        ... write extension namespace etc. ...
    }
    // else, version is < 1.3 and extensions are not allowed, the feature is not saved.

Another way to express "save only if 1.2 extended or in next version 1.3
or later" with `SvXMLExport& rExport` would be

    // Export only for 1.2 with extensions or 1.3 and later.
    SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
    if (eVersion > SvtSaveOptions::ODFSVER_012)
    {
        // For 1.2+ use coext namespace, for 1.3 use number namespace.
        rExport.AddAttribute(
                ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_CO_EXT : XML_NAMESPACE_NUMBER),
                XML_your_feature, aString );
    }

### Extension namespaces

Elements and attributes that are not defined in an ODF specification
yet, even if already submitted as a proposal to the OASIS ODF-TC,
**have** to be written with an extension namespace, otherwise validators
will complain about invalid elements or attributes. For example, instead
of `text:new-attribute` it needs to be `coext:new-attribute`. There are
predefined extension namespaces available, the only one that should be
used for new features is `XML_NAMESPACE_CO_EXT` that maps to the `coext`
of the example, which you would use instead of `XML_NAMESPACE_TEXT`.
(See `XML_NAMESPACE_EXT` in `include/xmloff/xmlnmspe.hxx` for
definitions of namespaces)

In the ODF reader prepare to import **both** namespaces, in this example
the `coext` that is written and the proposed future namespace, here
`text`. If you don't do this then the current version will not be able
to read documents that use the proposed attribute once it was accepted
by the OASIS ODF-TC. For example by using

    if (nPrefix == XML_NAMESPACE_CO_EXT || nPrefix == XML_NAMESPACE_TEXT)
    {
     ...
    }

or in an import (**!**) property map have two `SvXMLTokenMapEntry`

    { XML_NAMESPACE_CO_EXT, XML_NEW_ATTRIBUTE, XML_TOK_TEXT_NEW_ATTRIBUTE },
    { XML_NAMESPACE_TEXT,   XML_NEW_ATTRIBUTE, XML_TOK_TEXT_NEW_ATTRIBUTE },

  

### `SvXMLExportPropertyMapper`, Extensions in `XMLPropertyMapEntry` arrays

You may find yourself adding a new feature to an array that maps between
UNO properties and XML attributes.

Typically you need to add 2 entries, one with `CO_EXT` namespace and
`SvtSaveOptions::ODFSVER_FUTURE_EXTENDED` version, and the other with
some OASIS namespace and `SvtSaveOptions::ODFSVER_FUTURE_EXTENDED`
version; since LO 7.0, the `mbImportOnly` flag (the last member) doesn't
matter in this case. This means that on export the `CO_EXT` attribute
will be ignored in strict ODF versions and exported if the
`ODFSVER_EXTENDED` bit is set, the OASIS namespace attribute will always
be ignored on export, and both attributes will be imported.

When the extended attribute is accepted into an ODF Committee
Specification Draft, the version of both entries can be changed to the
corresponding `ODFSVER_xyz` value and any `mbImportOnly` flag removed;
this will prevent export of the `CO_EXT` attribute starting from this
version, and enable export of the OASIS namespace attribute starting
from this version.

## Collabora ODF extensions

- See the complete
[List_of_Collabora_ODF_Extensions.md](List_of_Collabora_ODF_Extensions.md)

## Collabora OpenFormula extensions

- See the complete
[List_of_Collabora_OpenFormula_Extensions.md](List_of_Collabora_OpenFormula_Extensions.md)

## ODF "implementation-defined" items in Collabora

- See the [List of ODF "implementation-defined" items in
 Collabora](List_of_Collabora_Implementation-Defined_Items.md)

## `settings.xml`

A (somewhat outdated) overview of the config-items supported by
Collabora Online and Collabora Office can be found
[in a spreadsheet in `online.git`](https://cgit.collaboraoffice.com/c/online/tree/engine/xmloff/documentation/LibreOffice_settings.xml_config-items.fods).

------------------------------------------------------------------------

## Footer

This page is based on a modified version of
[https://wiki.documentfoundation.org/index.php?title=Development/ODF_Implementer_Notes&oldid=855353](https://wiki.documentfoundation.org/index.php?title=Development/ODF_Implementer_Notes&oldid=855353)
and the Original Authors are: Balázs Meskó, Michael Stahl, Regina Henschel, Ilmari Lauhakangas, Dennis Roczek, Eike Rathke, Robinson Tryon, Miklos Vajna, Adolfo Jayme Barrientos, Andras Timar, Winfried Donkers, Dennis Roczek, Bjoern Michaelsen, Tamás Zolnai, Markus Mohrhard, Thorsten Behrens, Noel Power, Kohei Yoshida and Michael Meeks.

This page is licensed under the [Creative Commons Attribution-ShareAlike 3.0 Unported License](https://creativecommons.org/licenses/by-sa/3.0/legalcode.en). The original work has been modified.

