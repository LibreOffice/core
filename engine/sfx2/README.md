# Legacy Framework

`SFX` is the "old" framework, used for historical reasons.

An attempt of documentation of this module is located in `[git:sfx2/doc]`.

It contains base classes for document model, view and controller, used
by "old" applications like `sw`, `sc`, `sd` (while "new" applications
are based on the "new" UNO based framework in "framework").

The SFX framework is based on dispatching slots identified by integers
(`SlotIDs`) to `SfxShells`, and there is a dedicated IDL compiler (`svidl`)
involved that generates C++ slot headers from SDI files in modules' `sdi/`
subdirectory.

Documentation about SFX dispatch, SDI etc.:
<https://wiki.openoffice.org/wiki/Framework/Article/Implementation_of_the_Dispatch_API_In_SFX2>

Document load/save code is maintained in `[git:sfx2/source/doc/docfile.cxx`]
`SfxMedium` class, which handles all the twisty load and save corner cases.

`[git:sfx2/source/appl/sfxhelp.cxx]` Start procedure for the online
help viewer top level window; handling of help URL creation and
dispatch.

There are also some UNO services here that could really be implemented
anywhere, e.g. the `DocumentProperties` or `DocumentMetadataAccess`.

Notable files:
`sfx2/source/dialog/backingwindow.cxx` `Startcenter` buttons and the corresponding event handler.
