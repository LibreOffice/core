/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************************************************************/
extern SfxType0 aSfxInt32Item_Impl;
#ifdef SFX_TYPEMAP
SfxType0 aSfxInt32Item_Impl = 
{
    TYPE(SfxInt32Item), 0
};
#endif

/************************************************************/
extern SfxType0 aSfxBoolItem_Impl;
#ifdef SFX_TYPEMAP
SfxType0 aSfxBoolItem_Impl = 
{
    TYPE(SfxBoolItem), 0
};
#endif

/************************************************************/
extern SfxType0 aSfxUInt16Item_Impl;
#ifdef SFX_TYPEMAP
SfxType0 aSfxUInt16Item_Impl = 
{
    TYPE(SfxUInt16Item), 0
};
#endif

/************************************************************/
extern SfxType0 aSfxStringItem_Impl;
#ifdef SFX_TYPEMAP
SfxType0 aSfxStringItem_Impl = 
{
    TYPE(SfxStringItem), 0
};
#endif

/************************************************************/
extern SfxType0 aSfxObjectItem_Impl;
#ifdef SFX_TYPEMAP
SfxType0 aSfxObjectItem_Impl = 
{
    TYPE(SfxObjectItem), 0
};
#endif

/************************************************************/
extern SfxType0 aSfxVoidItem_Impl;
#ifdef SFX_TYPEMAP
SfxType0 aSfxVoidItem_Impl = 
{
    TYPE(SfxVoidItem), 0
};
#endif

/************************************************************/
#ifdef Form
#undef ShellClass
#undef Form
#define ShellClass Form
#endif

/************************************************************/
#ifdef FmFormShell
#undef ShellClass
#undef FmFormShell
#define ShellClass FmFormShell
SFX_ARGUMENTMAP(FmFormShell)
{
    SFX_ARGUMENT(FN_PARAM_1,"Position",SfxInt32Item),
    SFX_ARGUMENT(SID_FM_LEAVE_CREATE,"Leave",SfxBoolItem)
};

SFX_EXEC_STUB(FmFormShell,Execute)
SFX_STATE_STUB(FmFormShell,GetState)

SFX_SLOTMAP_ARG(FmFormShell)
{
// Slot Nr. 0 : 10593
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONFIG,SID_FM_CONFIG,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[1] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|0,
                0,
                SfxUInt16Item,
                0/*Offset*/, 0,".Config",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"Config" ),
// Slot Nr. 1 : 10594
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_PUSHBUTTON,SID_FM_PUSHBUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[2] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".Pushbutton",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"Pushbutton" ),
// Slot Nr. 2 : 10595
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RADIOBUTTON,SID_FM_RADIOBUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[3] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".RadioButton",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"RadioButton" ),
// Slot Nr. 3 : 10596
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CHECKBOX,SID_FM_CHECKBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[4] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".CheckBox",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"CheckBox" ),
// Slot Nr. 4 : 10597
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FIXEDTEXT,SID_FM_FIXEDTEXT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[5] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".Label",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"Label" ),
// Slot Nr. 5 : 10598
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_GROUPBOX,SID_FM_GROUPBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[6] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".GroupBox",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"GroupBox" ),
// Slot Nr. 6 : 10599
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_EDIT,SID_FM_EDIT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[7] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".Edit",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"Edit" ),
// Slot Nr. 7 : 10600
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_LISTBOX,SID_FM_LISTBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[8] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".ListBox",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ListBox" ),
// Slot Nr. 8 : 10601
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_COMBOBOX,SID_FM_COMBOBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[9] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".ComboBox",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ComboBox" ),
// Slot Nr. 9 : 10602
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_URLBUTTON,SID_FM_URLBUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[10] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".UrlButton",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"UrlButton" ),
// Slot Nr. 10 : 10603
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_DBGRID,SID_FM_DBGRID,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[11] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".Grid",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"Grid" ),
// Slot Nr. 11 : 10604
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_IMAGEBUTTON,SID_FM_IMAGEBUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[12] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".Imagebutton",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"Imagebutton" ),
// Slot Nr. 12 : 10605
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FILECONTROL,SID_FM_FILECONTROL,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[13] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".FileControl",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"FileControl" ),
// Slot Nr. 13 : 10613
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CTL_PROPERTIES,SID_FM_CTL_PROPERTIES,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[14] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".ControlProperties",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ControlProperties" ),
// Slot Nr. 14 : 10614
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_PROPERTIES,SID_FM_PROPERTIES,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[15] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_HASDIALOG|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".FormProperties",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"FormProperties" ),
// Slot Nr. 15 : 10615
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_TAB_DIALOG,SID_FM_TAB_DIALOG,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[16] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_HASDIALOG|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0,".TabDialog",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"TabDialog" ),
// Slot Nr. 16 : 10616
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_FIRST,SID_FM_RECORD_FIRST,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[17] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0/*Count*/,".FirstRecord",SFX_SLOT_METHOD|0,"FirstRecord" ),
// Slot Nr. 17 : 10617
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_NEXT,SID_FM_RECORD_NEXT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[18] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0/*Count*/,".NextRecord",SFX_SLOT_METHOD|0,"NextRecord" ),
// Slot Nr. 18 : 10618
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_PREV,SID_FM_RECORD_PREV,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[19] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0/*Count*/,".PrevRecord",SFX_SLOT_METHOD|0,"PrevRecord" ),
// Slot Nr. 19 : 10619
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_LAST,SID_FM_RECORD_LAST,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[20] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0/*Count*/,".LastRecord",SFX_SLOT_METHOD|0,"LastRecord" ),
// Slot Nr. 20 : 10620
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_NEW,SID_FM_RECORD_NEW,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[21] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0/*Count*/,".NewRecord",SFX_SLOT_METHOD|0,"NewRecord" ),
// Slot Nr. 21 : 10621
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_DELETE,SID_FM_RECORD_DELETE,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[22] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                0/*Offset*/, 0/*Count*/,".DeleteRecord",SFX_SLOT_METHOD|0,"DeleteRecord" ),
// Slot Nr. 22 : 10622
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_ABSOLUTE,SID_FM_RECORD_ABSOLUTE,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[23] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxInt32Item,
                0/*Offset*/, 1/*Count*/,".AbsoluteRecord",SFX_SLOT_METHOD|0,"AbsoluteRecord" ),
// Slot Nr. 23 : 10623
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_ADD_FIELD,SID_FM_ADD_FIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[26] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_HASDIALOG|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".AddField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"AddField" ),
// Slot Nr. 24 : 10624
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_TEXT,SID_FM_RECORD_TEXT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[25] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0/*Count*/,".RecText",SFX_SLOT_METHOD|0,"RecText" ),
// Slot Nr. 25 : 10625
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_FROM_TEXT,SID_FM_RECORD_FROM_TEXT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[51] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0/*Count*/,".RecFromText",SFX_SLOT_METHOD|0,"RecFromText" ),
// Slot Nr. 26 : 10626
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_TOTAL,SID_FM_RECORD_TOTAL,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[27] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxStringItem,
                1/*Offset*/, 0/*Count*/,".RecTotal",SFX_SLOT_METHOD|0,"RecTotal" ),
// Slot Nr. 27 : 10627
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_SAVE,SID_FM_RECORD_SAVE,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[28] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0/*Count*/,".RecSave",SFX_SLOT_METHOD|0,"RecSave" ),
// Slot Nr. 28 : 10629
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_DESIGN_MODE,SID_FM_DESIGN_MODE,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[29] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_HASDIALOG|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".SwitchControlDesignMode",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"SwitchControlDesignMode" ),
// Slot Nr. 29 : 10630
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_RECORD_UNDO,SID_FM_RECORD_UNDO,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[30] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0/*Count*/,".RecUndo",SFX_SLOT_METHOD|0,"RecUndo" ),
// Slot Nr. 30 : 10633
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_SHOW_FMEXPLORER,SID_FM_SHOW_FMEXPLORER,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[31] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".ShowFmExplorer",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ShowFmExplorer" ),
// Slot Nr. 31 : 10634
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FIELDS_CONTROL,SID_FM_FIELDS_CONTROL,0,
                0 ,&aFmFormShellSlots_Impl[32] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxObjectItem,
                1/*Offset*/, 0,".FieldController",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"FieldController" ),
// Slot Nr. 32 : 10635
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_SHOW_PROPERTIES,SID_FM_SHOW_PROPERTIES,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[33] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_HASDIALOG|SFX_SLOT_TOOLBOXCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".ShowProperties",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ShowProperties" ),
// Slot Nr. 33 : 10636
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_PROPERTY_CONTROL,SID_FM_PROPERTY_CONTROL,0,
                0 ,&aFmFormShellSlots_Impl[34] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxObjectItem,
                1/*Offset*/, 0,".PropertyController",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"PropertyController" ),
// Slot Nr. 34 : 10637
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FMEXPLORER_CONTROL,SID_FM_FMEXPLORER_CONTROL,0,
                0 ,&aFmFormShellSlots_Impl[35] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxObjectItem,
                1/*Offset*/, 0,".FmExplorerController",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"FmExplorerController" ),
// Slot Nr. 35 : 10703
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_SHOW_PROPERTY_BROWSER,SID_FM_SHOW_PROPERTY_BROWSER,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[36] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0/*Count*/,".ShowPropertyBrowser",SFX_SLOT_METHOD|0,"ShowPropertyBrowser" ),
// Slot Nr. 36 : 10704
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_DATEFIELD,SID_FM_DATEFIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[37] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".DateField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"DateField" ),
// Slot Nr. 37 : 10705
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_TIMEFIELD,SID_FM_TIMEFIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[38] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".TimeField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"TimeField" ),
// Slot Nr. 38 : 10706
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_NUMERICFIELD,SID_FM_NUMERICFIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[39] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".NumericField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"NumericField" ),
// Slot Nr. 39 : 10707
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CURRENCYFIELD,SID_FM_CURRENCYFIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[40] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".CurrencyField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"CurrencyField" ),
// Slot Nr. 40 : 10708
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_PATTERNFIELD,SID_FM_PATTERNFIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[41] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".PatternField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"PatternField" ),
// Slot Nr. 41 : 10709
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_OPEN_READONLY,SID_FM_OPEN_READONLY,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[42] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".OpenReadOnly",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"OpenReadOnly" ),
// Slot Nr. 42 : 10710
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_IMAGECONTROL,SID_FM_IMAGECONTROL,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[43] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0,".ImageControl",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ImageControl" ),
// Slot Nr. 43 : 10711
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_REMOVE_FILTER_SORT,SID_FM_REMOVE_FILTER_SORT,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[44] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".RemoveFilterSort",SFX_SLOT_METHOD|0,"RemoveFilterSort" ),
// Slot Nr. 44 : 10712
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_SORTUP,SID_FM_SORTUP,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[45] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".Sortup",SFX_SLOT_METHOD|0,"Sortup" ),
// Slot Nr. 45 : 10713
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_SORTDOWN,SID_FM_SORTDOWN,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[46] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".SortDown",SFX_SLOT_METHOD|0,"SortDown" ),
// Slot Nr. 46 : 10714
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_ORDERCRIT,SID_FM_ORDERCRIT,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[47] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_HASDIALOG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".OrderCrit",SFX_SLOT_METHOD|0,"OrderCrit" ),
// Slot Nr. 47 : 10716
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_AUTOFILTER,SID_FM_AUTOFILTER,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[48] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".AutoFilter",SFX_SLOT_METHOD|0,"AutoFilter" ),
// Slot Nr. 48 : 10723
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FORM_FILTERED,SID_FM_FORM_FILTERED,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[49] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                1/*Offset*/, 0/*Count*/,".FormFiltered",SFX_SLOT_METHOD|0,"FormFiltered" ),
// Slot Nr. 49 : 10724
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_REFRESH,SID_FM_REFRESH,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[50] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".Refresh",SFX_SLOT_METHOD|0,"Refresh" ),
// Slot Nr. 50 : 10725
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_SEARCH,SID_FM_SEARCH,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[52] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_HASDIALOG|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 0/*Count*/,".RecSearch",SFX_SLOT_METHOD|0,"RecSearch" ),
// Slot Nr. 51 : 10726
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_LEAVE_CREATE,SID_FM_LEAVE_CREATE,GID_EDIT,
                0 ,&aFmFormShellSlots_Impl[80] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR_STATE_NONE,
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|0,
                0,
                SfxVoidItem,
                1/*Offset*/, 1/*Count*/,".LeaveFMCreateMode",SFX_SLOT_METHOD|0,"LeaveFMCreateMode" ),
// Slot Nr. 52 : 10727
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_USE_WIZARDS,SID_FM_USE_WIZARDS,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[53] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".UseWizards",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"UseWizards" ),
// Slot Nr. 53 : 10728
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FORMATTEDFIELD,SID_FM_FORMATTEDFIELD,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[54] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".FormattedField",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"FormattedField" ),
// Slot Nr. 54 : 10729
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FILTER_START,SID_FM_FILTER_START,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[55] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                2/*Offset*/, 0/*Count*/,".FormFilter",SFX_SLOT_METHOD|0,"FormFilter" ),
// Slot Nr. 55 : 10730
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FILTER_EXIT,SID_FM_FILTER_EXIT,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[56] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                2/*Offset*/, 0/*Count*/,".FormFilterExit",SFX_SLOT_METHOD|0,"FormFilterExit" ),
// Slot Nr. 56 : 10731
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FILTER_EXECUTE,SID_FM_FILTER_EXECUTE,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[57] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                2/*Offset*/, 0/*Count*/,".FormFilterExecute",SFX_SLOT_METHOD|0,"FormFilterExecute" ),
// Slot Nr. 57 : 10732
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FILTER_NAVIGATOR,SID_FM_FILTER_NAVIGATOR,GID_TABLE,
                0 ,&aFmFormShellSlots_Impl[58] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                2/*Offset*/, 0/*Count*/,".FormFilterNavigator",SFX_SLOT_METHOD|0,"FormFilterNavigator" ),
// Slot Nr. 58 : 10733
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CHANGECONTROLTYPE,SID_FM_CHANGECONTROLTYPE,0,
                0 ,&aFmFormShellSlots_Impl[59] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ChangeControlType",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ChangeControlType" ),
// Slot Nr. 59 : 10734
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_EDIT,SID_FM_CONVERTTO_EDIT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[60] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToEdit",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToEdit" ),
// Slot Nr. 60 : 10735
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_BUTTON,SID_FM_CONVERTTO_BUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[61] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToButton",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToButton" ),
// Slot Nr. 61 : 10736
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_FIXEDTEXT,SID_FM_CONVERTTO_FIXEDTEXT,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[62] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToFixed",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToFixed" ),
// Slot Nr. 62 : 10737
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_LISTBOX,SID_FM_CONVERTTO_LISTBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[63] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToList",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToList" ),
// Slot Nr. 63 : 10738
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_CHECKBOX,SID_FM_CONVERTTO_CHECKBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[64] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToCheckBox",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToCheckBox" ),
// Slot Nr. 64 : 10739
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_RADIOBUTTON,SID_FM_CONVERTTO_RADIOBUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[65] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToRadio",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToRadio" ),
// Slot Nr. 65 : 10740
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_GROUPBOX,SID_FM_CONVERTTO_GROUPBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[66] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToGroup",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToGroup" ),
// Slot Nr. 66 : 10741
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_COMBOBOX,SID_FM_CONVERTTO_COMBOBOX,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[67] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToCombo",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToCombo" ),
// Slot Nr. 67 : 10742
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_GRID,SID_FM_CONVERTTO_GRID,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[68] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToGrid",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToGrid" ),
// Slot Nr. 68 : 10743
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_IMAGEBUTTON,SID_FM_CONVERTTO_IMAGEBUTTON,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[69] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToImageBtn",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToImageBtn" ),
// Slot Nr. 69 : 10744
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_FILECONTROL,SID_FM_CONVERTTO_FILECONTROL,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[70] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToFileControl",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToFileControl" ),
// Slot Nr. 70 : 10745
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_DATE,SID_FM_CONVERTTO_DATE,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[71] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToDate",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToDate" ),
// Slot Nr. 71 : 10746
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_TIME,SID_FM_CONVERTTO_TIME,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[72] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToTime",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToTime" ),
// Slot Nr. 72 : 10747
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_NUMERIC,SID_FM_CONVERTTO_NUMERIC,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[73] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToNumeric",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToNumeric" ),
// Slot Nr. 73 : 10748
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_CURRENCY,SID_FM_CONVERTTO_CURRENCY,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[74] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToCurrency",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToCurrency" ),
// Slot Nr. 74 : 10749
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_PATTERN,SID_FM_CONVERTTO_PATTERN,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[75] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToPattern",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToPattern" ),
// Slot Nr. 75 : 10750
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_IMAGECONTROL,SID_FM_CONVERTTO_IMAGECONTROL,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[76] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToImageControl",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToImageControl" ),
// Slot Nr. 76 : 10751
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_CONVERTTO_FORMATTED,SID_FM_CONVERTTO_FORMATTED,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[77] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".ConvertToFormatted",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"ConvertToFormatted" ),
// Slot Nr. 77 : 10752
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_FILTER_NAVIGATOR_CONTROL,SID_FM_FILTER_NAVIGATOR_CONTROL,0,
                0 ,&aFmFormShellSlots_Impl[78] /*Offset Next*/, 
                SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_VOLATILE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxObjectItem,
                2/*Offset*/, 0,".FmFilterNavigatorController",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"FmFilterNavigatorController" ),
// Slot Nr. 78 : 10761
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_VIEW_AS_GRID,SID_FM_VIEW_AS_GRID,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[79] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERITEM|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0/*Count*/,".ViewFormAsGrid",SFX_SLOT_METHOD|0,"ViewFormAsGrid" ),
// Slot Nr. 79 : 10763
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_AUTOCONTROLFOCUS,SID_FM_AUTOCONTROLFOCUS,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[0] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR(FmFormShell,GetState),
                SFX_SLOT_CACHABLE|SFX_SLOT_TOGGLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_ASYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_MENUCONFIG|SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxBoolItem,
                2/*Offset*/, 0,".AutoControlFocus",SFX_SLOT_PROPGET|SFX_SLOT_PROPSET|0,"AutoControlFocus" ),
// Slot Nr. 80 : 10767
    SFX_NEW_SLOT_ARG( FmFormShell,SID_FM_GRABCONTROLFOCUS,SID_FM_GRABCONTROLFOCUS,GID_CONTROLS,
                0 ,&aFmFormShellSlots_Impl[24] /*Offset Next*/, 
                SFX_STUB_PTR(FmFormShell,Execute),SFX_STUB_PTR_STATE_NONE,
                SFX_SLOT_CACHABLE|SFX_SLOT_AUTOUPDATE|SFX_SLOT_SYNCHRON|SFX_SLOT_RECORDPERSET|SFX_SLOT_ACCELCONFIG|SFX_SLOT_READONLYDOC|0,
                0,
                SfxVoidItem,
                2/*Offset*/, 0/*Count*/,".GrabControlFocus",SFX_SLOT_METHOD|0,"GrabControlFocus" )
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
