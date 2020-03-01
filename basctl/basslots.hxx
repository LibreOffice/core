/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxStringItem_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxBoolItem_Impl;
/************************************************************/
extern SfxType10 aSfxDocumentInfoItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType10 aSfxDocumentInfoItem_Impl =
    {
     createSfxPoolItem<SfxDocumentInfoItem>, &typeid(SfxDocumentInfoItem), 10, { {MID_DOCINFO_USEUSERDATA,"UseUserData"}, {MID_DOCINFO_DELETEUSERDATA,"DeleteUserData"}, {MID_DOCINFO_TITLE,"Title"}, {MID_DOCINFO_SUBJECT,"Subject"}, {MID_DOCINFO_KEYWORDS,"KeyWords"}, {MID_DOCINFO_DESCRIPTION,"Description"}, {MID_DOCINFO_AUTOLOADENABLED,"AutoReload"}, {MID_DOCINFO_AUTOLOADSECS,"AutoReloadTime"}, {MID_DOCINFO_AUTOLOADURL,"AutoReloadURL"}, {MID_DOCINFO_DEFAULTTARGET,"AutoReloadFrame"} }
    };
#endif

/************************************************************/
extern SfxType0 aSfxUnoAnyItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxUnoAnyItem_Impl =
    {
     createSfxPoolItem<SfxUnoAnyItem>, &typeid(SfxUnoAnyItem), 0
    };
#endif

/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt16Item_Impl;
/************************************************************/
extern SfxType23 aSvxSearchItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType23 aSvxSearchItem_Impl =
    {
     createSfxPoolItem<SvxSearchItem>, &typeid(SvxSearchItem), 23, { {MID_SEARCH_STYLEFAMILY,"StyleFamily"}, {MID_SEARCH_CELLTYPE,"CellType"}, {MID_SEARCH_ROWDIRECTION,"RowDirection"}, {MID_SEARCH_ALLTABLES,"AllTables"}, {MID_SEARCH_SEARCHFILTERED,"SearchFiltered"}, {MID_SEARCH_BACKWARD,"Backward"}, {MID_SEARCH_PATTERN,"Pattern"}, {MID_SEARCH_CONTENT,"Content"}, {MID_SEARCH_ASIANOPTIONS,"AsianOptions"}, {MID_SEARCH_ALGORITHMTYPE,"AlgorithmType"}, {MID_SEARCH_FLAGS,"SearchFlags"}, {MID_SEARCH_SEARCHSTRING,"SearchString"}, {MID_SEARCH_REPLACESTRING,"ReplaceString"}, {MID_SEARCH_LOCALE,"Locale"}, {MID_SEARCH_CHANGEDCHARS,"ChangedChars"}, {MID_SEARCH_DELETEDCHARS,"DeletedChars"}, {MID_SEARCH_INSERTEDCHARS,"InsertedChars"}, {MID_SEARCH_TRANSLITERATEFLAGS,"TransliterateFlags"}, {MID_SEARCH_COMMAND,"Command"}, {MID_SEARCH_STARTPOINTX,"SearchStartPointX"}, {MID_SEARCH_STARTPOINTY,"SearchStartPointY"}, {MID_SEARCH_SEARCHFORMATTED,"SearchFormatted"}, {MID_SEARCH_ALGORITHMTYPE2,"AlgorithmType2"} }
    };
#endif

/************************************************************/
extern SfxType0 aSfxMacroInfoItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxMacroInfoItem_Impl =
    {
     createSfxPoolItem<SfxMacroInfoItem>, &typeid(SfxMacroInfoItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSbxItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSbxItem_Impl =
    {
     createSfxPoolItem<SbxItem>, &typeid(SbxItem), 0
    };
#endif

/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt32Item_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxVoidItem_Impl;
/************************************************************/
extern SfxType2 aSvxSizeItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType2 aSvxSizeItem_Impl =
    {
     createSfxPoolItem<SvxSizeItem>, &typeid(SvxSizeItem), 2, { {MID_SIZE_WIDTH,"Width"}, {MID_SIZE_HEIGHT,"Height"} }
    };
#endif

/************************************************************/
#ifdef ShellClass_BasicIDEView
#undef ShellClass
#undef ShellClass_BasicIDEView
#define ShellClass BasicIDEView
#endif

/************************************************************/
#ifdef ShellClass_basctl_Shell
#undef ShellClass
#undef ShellClass_basctl_Shell
#define ShellClass basctl_Shell
static SfxFormalArgument abasctl_ShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "URL", SID_FILE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", SID_FILTER_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Password", SID_PASSWORD },
     { (const SfxType*) &aSfxBoolItem_Impl, "PasswordInteraction", SID_PASSWORDINTERACTION },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterOptions", SID_FILE_FILTEROPTIONS },
     { (const SfxType*) &aSfxStringItem_Impl, "VersionComment", SID_DOCINFO_COMMENTS },
     { (const SfxType*) &aSfxStringItem_Impl, "VersionAuthor", SID_DOCINFO_AUTHOR },
     { (const SfxType*) &aSfxBoolItem_Impl, "Overwrite", SID_OVERWRITE },
     { (const SfxType*) &aSfxBoolItem_Impl, "Unpacked", SID_UNPACK },
     { (const SfxType*) &aSfxBoolItem_Impl, "SaveTo", SID_SAVETO },
     { (const SfxType*) &aSfxBoolItem_Impl, "NoFileSync", SID_NO_FILE_SYNC },
     { (const SfxType*) &aSfxBoolItem_Impl, "NoThumbnail", SID_NO_THUMBNAIL },
     { (const SfxType*) &aSfxStringItem_Impl, "VersionComment", SID_DOCINFO_COMMENTS },
     { (const SfxType*) &aSfxStringItem_Impl, "Author", SID_DOCINFO_AUTHOR },
     { (const SfxType*) &aSfxBoolItem_Impl, "DontTerminateEdit", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "NoFileSync", SID_NO_FILE_SYNC },
     { (const SfxType*) &aSfxDocumentInfoItem_Impl, "Properties", SID_DOCINFO },
     { (const SfxType*) &aSfxUnoAnyItem_Impl, "AsyncFunc", FN_PARAM_2 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSfxBoolItem_Impl, "Quiet", SID_SEARCH_QUIET },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSfxBoolItem_Impl, "Quiet", SID_SEARCH_QUIET },
     { (const SfxType*) &aSfxMacroInfoItem_Impl, "aMacroInfo", SID_BASICIDE_ARG_MACROINFO },
     { (const SfxType*) &aSfxMacroInfoItem_Impl, "aMacroInfo", SID_BASICIDE_ARG_MACROINFO },
     { (const SfxType*) &aSbxItem_Impl, "Sbx", SID_BASICIDE_ARG_SBX },
     { (const SfxType*) &aSfxUInt16Item_Impl, "TabId", SID_BASICIDE_ARG_TABID },
     { (const SfxType*) &aSfxStringItem_Impl, "NewName", SID_BASICIDE_ARG_MODULENAME },
     { (const SfxType*) &aSfxMacroInfoItem_Impl, "aMacroInfo", SID_BASICIDE_ARG_MACROINFO },
     { (const SfxType*) &aSfxMacroInfoItem_Impl, "aMacroInfo", SID_BASICIDE_ARG_MACROINFO },
     { (const SfxType*) &aSfxUnoAnyItem_Impl, "Document", SID_BASICIDE_ARG_DOCUMENT_MODEL },
     { (const SfxType*) &aSfxStringItem_Impl, "LibName", SID_BASICIDE_ARG_LIBNAME },
     { (const SfxType*) &aSbxItem_Impl, "Sbx", SID_BASICIDE_ARG_SBX },
     { (const SfxType*) &aSbxItem_Impl, "Sbx", SID_BASICIDE_ARG_SBX },
     { (const SfxType*) &aSfxUnoAnyItem_Impl, "Document", SID_BASICIDE_ARG_DOCUMENT_MODEL },
     { (const SfxType*) &aSfxStringItem_Impl, "LibName", SID_BASICIDE_ARG_LIBNAME },
     { (const SfxType*) &aSfxUnoAnyItem_Impl, "Document", SID_BASICIDE_ARG_DOCUMENT_MODEL },
     { (const SfxType*) &aSfxStringItem_Impl, "LibName", SID_BASICIDE_ARG_LIBNAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Document", SID_BASICIDE_ARG_DOCUMENT },
     { (const SfxType*) &aSfxStringItem_Impl, "LibName", SID_BASICIDE_ARG_LIBNAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_BASICIDE_ARG_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Type", SID_BASICIDE_ARG_TYPE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Line", SID_BASICIDE_ARG_LINE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Column1", SID_BASICIDE_ARG_COLUMN1 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Column2", SID_BASICIDE_ARG_COLUMN2 }
    };

SFX_EXEC_STUB(basctl_Shell,ExecuteGlobal)
SFX_STATE_STUB(basctl_Shell,GetState)
SFX_EXEC_STUB(basctl_Shell,ExecuteCurrent)
SFX_EXEC_STUB(basctl_Shell,ExecuteBasic)
SFX_EXEC_STUB(basctl_Shell,ExecuteDialog)
SFX_EXEC_STUB(basctl_Shell,ExecuteSearch)

static SfxSlot abasctl_ShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5502
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SAVEASDOC,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 12/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveAs" ),
     // Slot Nr. 1 : 5505
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SAVEDOC,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       12/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Save" ),
     // Slot Nr. 2 : 5535
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_DOCINFO,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetDocumentProperties" ),
     // Slot Nr. 3 : 5584
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_DOC_MODIFIED,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       18/*Offset*/, 0, SfxSlotMode::NONE,"ModifiedStatus" ),
     // Slot Nr. 4 : 5620
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_NEWWINDOW,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       18/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewWindow" ),
     // Slot Nr. 5 : 5700
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_REDO,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       18/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 6 : 5701
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_UNDO,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       20/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 7 : 5710
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_CUT,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 8 : 5711
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_COPY,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 9 : 5712
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_PASTE,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 10 : 5713
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_DELETE,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 11 : 5714
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BACKSPACE,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Backspace" ),
     // Slot Nr. 12 : 5723
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SELECTALL,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 13 : 5951
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICLOAD,SfxGroupId::Insert,
                       &abasctl_ShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LoadBasic" ),
     // Slot Nr. 14 : 5953
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICSAVEAS,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveBasicAs" ),
     // Slot Nr. 15 : 5954
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICCOMPILE,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CompileBasic" ),
     // Slot Nr. 16 : 5955
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICRUN,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RunBasic" ),
     // Slot Nr. 17 : 5956
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICSTEPINTO,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicStepInto" ),
     // Slot Nr. 18 : 5957
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICSTEPOVER,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicStepOver" ),
     // Slot Nr. 19 : 5958
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICSTOP,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicStop" ),
     // Slot Nr. 20 : 5963
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICSTEPOUT,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicStepOut" ),
     // Slot Nr. 21 : 5965
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_EXPORT_DIALOG,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExportDialog" ),
     // Slot Nr. 22 : 5966
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_IMPORT_DIALOG,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ImportDialog" ),
     // Slot Nr. 23 : 6643
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SIGNATURE,SfxGroupId::Document,
                       &abasctl_ShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Signature" ),
     // Slot Nr. 24 : 6724
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_GOTOLINE,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"GotoLine" ),
     // Slot Nr. 25 : 6725
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SHOWLINES,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"ShowLines" ),
     // Slot Nr. 26 : 10144
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_CHOOSE_CONTROLS,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"ChooseControls" ),
     // Slot Nr. 27 : 10146
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_PUSHBUTTON,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"InsertPushbutton" ),
     // Slot Nr. 28 : 10147
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_RADIOBUTTON,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Radiobutton" ),
     // Slot Nr. 29 : 10148
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_CHECKBOX,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Checkbox" ),
     // Slot Nr. 30 : 10189
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FIXEDTEXT,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"InsertFixedText" ),
     // Slot Nr. 31 : 10190
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_GROUPBOX,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Groupbox" ),
     // Slot Nr. 32 : 10191
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_EDIT,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"InsertEdit" ),
     // Slot Nr. 33 : 10192
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_LISTBOX,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"InsertListbox" ),
     // Slot Nr. 34 : 10193
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_COMBOBOX,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Combobox" ),
     // Slot Nr. 35 : 10194
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_SPINBUTTON,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Spinbutton" ),
     // Slot Nr. 36 : 10195
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_HSCROLLBAR,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"HScrollbar" ),
     // Slot Nr. 37 : 10196
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_VSCROLLBAR,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"VScrollbar" ),
     // Slot Nr. 38 : 10199
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_SELECT,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"SelectMode" ),
     // Slot Nr. 39 : 10200
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_DIALOG_TESTMODE,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TestMode" ),
     // Slot Nr. 40 : 10221
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_ATTR_INSERT,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"InsertMode" ),
     // Slot Nr. 41 : 10224
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_ATTR_SIZE,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSizeItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"Size" ),
     // Slot Nr. 42 : 10281
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SEARCH_OPTIONS,SfxGroupId::Intern,
                       &abasctl_ShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteSearch),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"SearchOptions" ),
     // Slot Nr. 43 : 10291
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SEARCH_ITEM,SfxGroupId::NONE,
                       &abasctl_ShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteSearch),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSearchItem,
                       24/*Offset*/, 0, SfxSlotMode::NONE,"SearchProperties" ),
     // Slot Nr. 44 : 10500
     SFX_NEW_SLOT_ARG( basctl_Shell,FID_SEARCH_ON,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[103] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SearchOn" ),
     // Slot Nr. 45 : 10501
     SFX_NEW_SLOT_ARG( basctl_Shell,FID_SEARCH_OFF,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SearchOff" ),
     // Slot Nr. 46 : 10502
     SFX_NEW_SLOT_ARG( basctl_Shell,FID_SEARCH_NOW,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteSearch),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       26/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExecuteSearch" ),
     // Slot Nr. 47 : 10503
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_REPEAT_SEARCH,SfxGroupId::Edit,
                       &abasctl_ShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteSearch),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RepeatSearch" ),
     // Slot Nr. 48 : 10753
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_GRIDCONTROL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertGridControl" ),
     // Slot Nr. 49 : 10926
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_IMAGECONTROL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertImageControl" ),
     // Slot Nr. 50 : 10927
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_PROGRESSBAR,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"ProgressBar" ),
     // Slot Nr. 51 : 10928
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_HFIXEDLINE,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"HFixedLine" ),
     // Slot Nr. 52 : 10929
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_VFIXEDLINE,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"VFixedLine" ),
     // Slot Nr. 53 : 10936
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_DATEFIELD,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"AddDateField" ),
     // Slot Nr. 54 : 10937
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_TIMEFIELD,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertTimeField" ),
     // Slot Nr. 55 : 10938
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_NUMERICFIELD,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertNumericField" ),
     // Slot Nr. 56 : 10939
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_CURRENCYFIELD,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertCurrencyField" ),
     // Slot Nr. 57 : 10940
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORMATTEDFIELD,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormattedField" ),
     // Slot Nr. 58 : 10941
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_PATTERNFIELD,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertPatternField" ),
     // Slot Nr. 59 : 10942
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FILECONTROL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFileControl" ),
     // Slot Nr. 60 : 10943
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_SHOW_PROPERTYBROWSER,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"ShowPropBrowser" ),
     // Slot Nr. 61 : 11037
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_HYPERLINKCONTROL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertHyperlinkControl" ),
     // Slot Nr. 62 : 11063
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_TREECONTROL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertTreeControl" ),
     // Slot Nr. 63 : 11106
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_RADIO,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormRadio" ),
     // Slot Nr. 64 : 11107
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_CHECK,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormCheck" ),
     // Slot Nr. 65 : 11108
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_LIST,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormList" ),
     // Slot Nr. 66 : 11109
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_COMBO,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormCombo" ),
     // Slot Nr. 67 : 11110
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_SPIN,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormSpin" ),
     // Slot Nr. 68 : 11111
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_VSCROLL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormVScroll" ),
     // Slot Nr. 69 : 11112
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_INSERT_FORM_HSCROLL,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteDialog),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"InsertFormHScroll" ),
     // Slot Nr. 70 : 30768
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_TOGGLEBRKPNT,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleBreakPoint" ),
     // Slot Nr. 71 : 30769
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_ADDWATCH,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AddWatch" ),
     // Slot Nr. 72 : 30770
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_CHOOSEMACRO,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChooseMacro" ),
     // Slot Nr. 73 : 30771
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_EDITMACRO,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditMacro" ),
     // Slot Nr. 74 : 30772
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_CREATEMACRO,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateMacro" ),
     // Slot Nr. 75 : 30773
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_MODULEDLG,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ModuleDialog" ),
     // Slot Nr. 76 : 30774
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_OBJCAT,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectCatalog" ),
     // Slot Nr. 77 : 30775
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_REMOVEWATCH,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RemoveWatch" ),
     // Slot Nr. 78 : 30777
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_SHOWSBX,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowSbx" ),
     // Slot Nr. 79 : 30778
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_HIDECURPAGE,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideCurPage" ),
     // Slot Nr. 80 : 30779
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_NAMECHANGEDONTAB,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NameChangedOnTab" ),
     // Slot Nr. 81 : 30780
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_UPDATEMODULESOURCE,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UpdateModuleSource" ),
     // Slot Nr. 82 : 30781
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_STOREMODULESOURCE,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StoreModuleSource" ),
     // Slot Nr. 83 : 30782
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_MATCHGROUP,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MatchGroup" ),
     // Slot Nr. 84 : 30784
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_STOREALLMODULESOURCES,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StoreAllModuleSources" ),
     // Slot Nr. 85 : 30785
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_UPDATEALLMODULESOURCES,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UpdateAllModuleSources" ),
     // Slot Nr. 86 : 30786
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_LIBSELECTED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       37/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LibSelect" ),
     // Slot Nr. 87 : 30787
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_LIBSELECTOR,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"LibSelector" ),
     // Slot Nr. 88 : 30788
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_NEWDIALOG,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewDialog" ),
     // Slot Nr. 89 : 30789
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_NEWMODULE,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewModule" ),
     // Slot Nr. 90 : 30790
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_DELETECURRENT,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteCurrent" ),
     // Slot Nr. 91 : 30791
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_RENAMECURRENT,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteCurrent),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenameCurrent" ),
     // Slot Nr. 92 : 30792
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_SBXDELETED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SbxDeleted" ),
     // Slot Nr. 93 : 30793
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_SBXINSERTED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SbxInserted" ),
     // Slot Nr. 94 : 30794
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_SBXRENAMED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SbxRenamed" ),
     // Slot Nr. 95 : 30795
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_LIBLOADED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       41/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LibLoaded" ),
     // Slot Nr. 96 : 30796
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_LIBREMOVED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       43/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LibRemoved" ),
     // Slot Nr. 97 : 30804
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_BRKPNTSCHANGED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BreakPointsChanged" ),
     // Slot Nr. 98 : 30806
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_STAT_POS,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"StatusGetPosition" ),
     // Slot Nr. 99 : 30807
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_STAT_DATE,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"StatusGetDate" ),
     // Slot Nr. 100 : 30808
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_STAT_TITLE,SfxGroupId::View,
                       &abasctl_ShellSlots_Impl[101] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"StatusGetTitle" ),
     // Slot Nr. 101 : 30810
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_MANAGEBRKPNTS,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[102] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ManageBreakPoints" ),
     // Slot Nr. 102 : 30811
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_TOGGLEBRKPNTENABLED,SfxGroupId::Macro,
                       &abasctl_ShellSlots_Impl[104] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteBasic),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleBreakPointEnabled" ),
     // Slot Nr. 103 : 30812
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_SHOWWINDOW,SfxGroupId::Application,
                       &abasctl_ShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 7/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicIDEShowWindow" ),
     // Slot Nr. 104 : 30820
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_MANAGE_LANG,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[105] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       52/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ManageLanguage" ),
     // Slot Nr. 105 : 30821
     SFX_NEW_SLOT_ARG( basctl_Shell,SID_BASICIDE_CURRENT_LANG,SfxGroupId::Controls,
                       &abasctl_ShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(basctl_Shell,ExecuteGlobal),SFX_STUB_PTR(basctl_Shell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       52/*Offset*/, 0, SfxSlotMode::NONE,"CurrentLanguage" )
    };
#endif

/************************************************************/
#ifdef ShellClass_BasicIDEDocument
#undef ShellClass
#undef ShellClass_BasicIDEDocument
#define ShellClass BasicIDEDocument
#endif

/************************************************************/
#ifdef ShellClass_basctl_DocShell
#undef ShellClass
#undef ShellClass_basctl_DocShell
#define ShellClass basctl_DocShell
static SfxFormalArgument abasctl_DocShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };


static SfxSlot abasctl_DocShellSlots_Impl[] =
    {
     SFX_SLOT_ARG(basctl_DocShell, 0, SfxGroupId::NONE, SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,SfxSlotMode::NONE, SfxVoidItem, 0, 0, "", SfxSlotMode::NONE )

    };
#endif
