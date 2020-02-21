/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxStringItem_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxVoidItem_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt16Item_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxBoolItem_Impl;
/************************************************************/
extern SfxType0 aSfxStringListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxStringListItem_Impl =
    {
     createSfxPoolItem<SfxStringListItem>, &typeid(SfxStringListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSfxInt16Item_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxInt16Item_Impl =
    {
     createSfxPoolItem<SfxInt16Item>, &typeid(SfxInt16Item), 0
    };
#endif

/************************************************************/
extern SfxType3 aSvxZoomItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType3 aSvxZoomItem_Impl =
    {
     createSfxPoolItem<SvxZoomItem>, &typeid(SvxZoomItem), 3, { {MID_VALUE,"Value"}, {MID_VALUESET,"ValueSet"}, {MID_TYPE,"Type"} }
    };
#endif

/************************************************************/
extern SfxType4 aSvxZoomSliderItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSvxZoomSliderItem_Impl =
    {
     createSfxPoolItem<SvxZoomSliderItem>, &typeid(SvxZoomSliderItem), 4, { {MID_ZOOMSLIDER_CURRENTZOOM,"CurrentZoom"}, {MID_ZOOMSLIDER_SNAPPINGPOINTS,"SnappingPoints"}, {MID_ZOOMSLIDER_MINZOOM,"SvxMinZoom"}, {MID_ZOOMSLIDER_MAXZOOM,"SvxMaxZoom"} }
    };
#endif

/************************************************************/
#ifdef ShellClass_StarMath
#undef ShellClass
#undef ShellClass_StarMath
#define ShellClass StarMath
#endif

/************************************************************/
#ifdef ShellClass_SmModule
#undef ShellClass
#undef ShellClass_SmModule
#define ShellClass SmModule
static SfxFormalArgument aSmModuleArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", SID_CFGFILE }
    };

SFX_STATE_STUB(SmModule,GetState)

static SfxSlot aSmModuleSlots_Impl[] =
    {
     // Slot Nr. 0 : 5926
     SFX_NEW_SLOT_ARG( SmModule,SID_CONFIGEVENT,SfxGroupId::Options,
                       &aSmModuleSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(SmModule,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LoadEvents" )
    };
#endif

/************************************************************/
#ifdef ShellClass_FormulaDocument
#undef ShellClass
#undef ShellClass_FormulaDocument
#define ShellClass FormulaDocument
#endif

/************************************************************/
#ifdef ShellClass_SmDocShell
#undef ShellClass
#undef ShellClass_SmDocShell
#define ShellClass SmDocShell
static SfxFormalArgument aSmDocShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "TemplateRegion", SID_TEMPLATE_REGIONNAME },
     { (const SfxType*) &aSfxStringItem_Impl, "TemplateName", SID_TEMPLATE_NAME },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE }
    };

SFX_STATE_STUB(SmDocShell,GetState)
SFX_EXEC_STUB(SmDocShell,Execute)

static SfxSlot aSmDocShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5538
     SFX_NEW_SLOT_ARG( SmDocShell,SID_DOCTEMPLATE,SfxGroupId::Template,
                       &aSmDocShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveAsTemplate" ),
     // Slot Nr. 1 : 5700
     SFX_NEW_SLOT_ARG( SmDocShell,SID_REDO,SfxGroupId::Edit,
                       &aSmDocShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 2 : 5701
     SFX_NEW_SLOT_ARG( SmDocShell,SID_UNDO,SfxGroupId::Edit,
                       &aSmDocShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       4/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 3 : 10923
     SFX_NEW_SLOT_ARG( SmDocShell,SID_GETUNDOSTRINGS,SfxGroupId::Edit,
                       &aSmDocShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"GetUndoStrings" ),
     // Slot Nr. 4 : 10924
     SFX_NEW_SLOT_ARG( SmDocShell,SID_GETREDOSTRINGS,SfxGroupId::Edit,
                       &aSmDocShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"GetRedoStrings" ),
     // Slot Nr. 5 : 30306
     SFX_NEW_SLOT_ARG( SmDocShell,SID_FONT,SfxGroupId::Format,
                       &aSmDocShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeFont" ),
     // Slot Nr. 6 : 30307
     SFX_NEW_SLOT_ARG( SmDocShell,SID_FONTSIZE,SfxGroupId::Format,
                       &aSmDocShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeFontSize" ),
     // Slot Nr. 7 : 30308
     SFX_NEW_SLOT_ARG( SmDocShell,SID_DISTANCE,SfxGroupId::Format,
                       &aSmDocShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeDistance" ),
     // Slot Nr. 8 : 30309
     SFX_NEW_SLOT_ARG( SmDocShell,SID_ALIGN,SfxGroupId::Format,
                       &aSmDocShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeAlignment" ),
     // Slot Nr. 9 : 30311
     SFX_NEW_SLOT_ARG( SmDocShell,SID_AUTO_REDRAW,SfxGroupId::View,
                       &aSmDocShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"RedrawAutomatic" ),
     // Slot Nr. 10 : 30313
     SFX_NEW_SLOT_ARG( SmDocShell,SID_TEXTMODE,SfxGroupId::View,
                       &aSmDocShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"Textmode" ),
     // Slot Nr. 11 : 30356
     SFX_NEW_SLOT_ARG( SmDocShell,SID_TEXT,SfxGroupId::View,
                       &aSmDocShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"ConfigName" ),
     // Slot Nr. 12 : 30357
     SFX_NEW_SLOT_ARG( SmDocShell,SID_GAPHIC_SM,SfxGroupId::Math,
                       &aSmDocShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"Graphic" ),
     // Slot Nr. 13 : 30363
     SFX_NEW_SLOT_ARG( SmDocShell,SID_LOADSYMBOLS,SfxGroupId::Options,
                       &aSmDocShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LoadSymbols" ),
     // Slot Nr. 14 : 30364
     SFX_NEW_SLOT_ARG( SmDocShell,SID_SAVESYMBOLS,SfxGroupId::Options,
                       &aSmDocShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveSymbols" ),
     // Slot Nr. 15 : 30366
     SFX_NEW_SLOT_ARG( SmDocShell,SID_MODIFYSTATUS,SfxGroupId::View,
                       &aSmDocShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(SmDocShell,Execute),SFX_STUB_PTR(SmDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       6/*Offset*/, 0, SfxSlotMode::NONE,"ModifyStatus" )
    };
#endif

/************************************************************/
#ifdef ShellClass_FormulaView
#undef ShellClass
#undef ShellClass_FormulaView
#define ShellClass FormulaView
#endif

/************************************************************/
#ifdef ShellClass_SmViewShell
#undef ShellClass
#undef ShellClass_SmViewShell
#define ShellClass SmViewShell
static SfxFormalArgument aSmViewShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_IMPORT_FORMULA },
     { (const SfxType*) &aSfxStringItem_Impl, "Filter", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Text", SID_INSERTCOMMANDTEXT }
    };

SFX_EXEC_STUB(SmViewShell,Execute)
SFX_STATE_STUB(SmViewShell,GetState)

static SfxSlot aSmViewShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5710
     SFX_NEW_SLOT_ARG( SmViewShell,SID_CUT,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 1 : 5711
     SFX_NEW_SLOT_ARG( SmViewShell,SID_COPY,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 2 : 5712
     SFX_NEW_SLOT_ARG( SmViewShell,SID_PASTE,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 3 : 5713
     SFX_NEW_SLOT_ARG( SmViewShell,SID_DELETE,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 4 : 5720
     SFX_NEW_SLOT_ARG( SmViewShell,SID_SELECT,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Select" ),
     // Slot Nr. 5 : 5792
     SFX_NEW_SLOT_ARG( SmViewShell,SID_UNICODE_NOTATION_TOGGLE,SfxGroupId::Options,
                       &aSmViewShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UnicodeNotationToggle" ),
     // Slot Nr. 6 : 10000
     SFX_NEW_SLOT_ARG( SmViewShell,SID_ATTR_ZOOM,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"Zoom" ),
     // Slot Nr. 7 : 11065
     SFX_NEW_SLOT_ARG( SmViewShell,SID_ATTR_ZOOMSLIDER,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomSliderItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"ZoomSlider" ),
     // Slot Nr. 8 : 30257
     SFX_NEW_SLOT_ARG( SmViewShell,SID_NEXTERR,SfxGroupId::Navigator,
                       &aSmViewShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NextError" ),
     // Slot Nr. 9 : 30258
     SFX_NEW_SLOT_ARG( SmViewShell,SID_PREVERR,SfxGroupId::Navigator,
                       &aSmViewShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PrevError" ),
     // Slot Nr. 10 : 30259
     SFX_NEW_SLOT_ARG( SmViewShell,SID_NEXTMARK,SfxGroupId::Navigator,
                       &aSmViewShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NextMark" ),
     // Slot Nr. 11 : 30260
     SFX_NEW_SLOT_ARG( SmViewShell,SID_PREVMARK,SfxGroupId::Navigator,
                       &aSmViewShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PrevMark" ),
     // Slot Nr. 12 : 30261
     SFX_NEW_SLOT_ARG( SmViewShell,SID_SYMBOLS_CATALOGUE,SfxGroupId::Options,
                       &aSmViewShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SymbolCatalogue" ),
     // Slot Nr. 13 : 30266
     SFX_NEW_SLOT_ARG( SmViewShell,SID_ZOOMIN,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomIn" ),
     // Slot Nr. 14 : 30267
     SFX_NEW_SLOT_ARG( SmViewShell,SID_ZOOMOUT,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomOut" ),
     // Slot Nr. 15 : 30268
     SFX_NEW_SLOT_ARG( SmViewShell,SID_DRAW,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Draw" ),
     // Slot Nr. 16 : 30271
     SFX_NEW_SLOT_ARG( SmViewShell,SID_FORMULACURSOR,SfxGroupId::Math,
                       &aSmViewShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormelCursor" ),
     // Slot Nr. 17 : 30314
     SFX_NEW_SLOT_ARG( SmViewShell,SID_IMPORT_FORMULA,SfxGroupId::Insert,
                       &aSmViewShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ImportFormula" ),
     // Slot Nr. 18 : 30315
     SFX_NEW_SLOT_ARG( SmViewShell,SID_IMPORT_MATHML_CLIPBOARD,SfxGroupId::Insert,
                       &aSmViewShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ImportMathMLClipboard" ),
     // Slot Nr. 19 : 30360
     SFX_NEW_SLOT_ARG( SmViewShell,SID_INSERTSPECIAL,SfxGroupId::Insert,
                       &aSmViewShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertConfigName" ),
     // Slot Nr. 20 : 30362
     SFX_NEW_SLOT_ARG( SmViewShell,SID_INSERTCOMMANDTEXT,SfxGroupId::Insert,
                       &aSmViewShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertCommandText" ),
     // Slot Nr. 21 : 30367
     SFX_NEW_SLOT_ARG( SmViewShell,SID_TEXTSTATUS,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"TextStatus" ),
     // Slot Nr. 22 : 30373
     SFX_NEW_SLOT_ARG( SmViewShell,SID_COPYOBJECT,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CopyObject" ),
     // Slot Nr. 23 : 30374
     SFX_NEW_SLOT_ARG( SmViewShell,SID_PASTEOBJECT,SfxGroupId::Edit,
                       &aSmViewShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteObject" ),
     // Slot Nr. 24 : 30377
     SFX_NEW_SLOT_ARG( SmViewShell,SID_GETEDITTEXT,SfxGroupId::Math,
                       &aSmViewShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetPaperSize" ),
     // Slot Nr. 25 : 30378
     SFX_NEW_SLOT_ARG( SmViewShell,SID_CMDBOXWINDOW,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommandWindow" ),
     // Slot Nr. 26 : 30382
     SFX_NEW_SLOT_ARG( SmViewShell,SID_ELEMENTSDOCKINGWINDOW,SfxGroupId::View,
                       &aSmViewShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(SmViewShell,Execute),SFX_STUB_PTR(SmViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ElementsDockingWindow" )
    };
#endif
