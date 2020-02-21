/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxStringItem_Impl;
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
extern SfxType0 aSfxUnoFrameItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxUnoFrameItem_Impl =
    {
     createSfxPoolItem<SfxUnoFrameItem>, &typeid(SfxUnoFrameItem), 0
    };
#endif

/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxBoolItem_Impl;
/************************************************************/
extern SfxType0 aSfxLockBytesItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxLockBytesItem_Impl =
    {
     createSfxPoolItem<SfxLockBytesItem>, &typeid(SfxLockBytesItem), 0
    };
#endif

/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxVoidItem_Impl;
/************************************************************/
extern SfxType0 aSvxLanguageItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxLanguageItem_Impl =
    {
     createSfxPoolItem<SvxLanguageItem>, &typeid(SvxLanguageItem), 0
    };
#endif

/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt16Item_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt32Item_Impl;
/************************************************************/
extern SfxType0 aSfxGlobalNameItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxGlobalNameItem_Impl =
    {
     createSfxPoolItem<SfxGlobalNameItem>, &typeid(SfxGlobalNameItem), 0
    };
#endif

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
extern SfxType0 aSfxByteItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxByteItem_Impl =
    {
     createSfxPoolItem<SfxByteItem>, &typeid(SfxByteItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxColorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxColorItem_Impl =
    {
     createSfxPoolItem<SvxColorItem>, &typeid(SvxColorItem), 0
    };
#endif

/************************************************************/
extern SfxType8 aSvxLRSpaceItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType8 aSvxLRSpaceItem_Impl =
    {
     createSfxPoolItem<SvxLRSpaceItem>, &typeid(SvxLRSpaceItem), 8, { {MID_L_MARGIN,"LeftMargin"}, {MID_TXT_LMARGIN,"TextLeftMargin"}, {MID_R_MARGIN,"RightMargin"}, {MID_L_REL_MARGIN,"LeftRelMargin"}, {MID_R_REL_MARGIN,"RightRelMargin"}, {MID_FIRST_LINE_INDENT,"FirstLineIndent"}, {MID_FIRST_LINE_REL_INDENT,"FirstLineRelIdent"}, {MID_FIRST_AUTO,"AutoFirst"} }
    };
#endif

/************************************************************/
extern SfxType0 aXFillColorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFillColorItem_Impl =
    {
     createSfxPoolItem<XFillColorItem>, &typeid(XFillColorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxDoubleItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxDoubleItem_Impl =
    {
     createSfxPoolItem<SvxDoubleItem>, &typeid(SvxDoubleItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXLineWidthItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXLineWidthItem_Impl =
    {
     createSfxPoolItem<XLineWidthItem>, &typeid(XLineWidthItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXLineColorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXLineColorItem_Impl =
    {
     createSfxPoolItem<XLineColorItem>, &typeid(XLineColorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxGalleryItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxGalleryItem_Impl =
    {
     createSfxPoolItem<SvxGalleryItem>, &typeid(SvxGalleryItem), 0
    };
#endif

/************************************************************/
extern SfxType2 aSvxLongULSpaceItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType2 aSvxLongULSpaceItem_Impl =
    {
     createSfxPoolItem<SvxLongULSpaceItem>, &typeid(SvxLongULSpaceItem), 2, { {MID_UPPER,"Upper"}, {MID_LOWER,"Lower"} }
    };
#endif

/************************************************************/
extern SfxType1 aXLineStartItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType1 aXLineStartItem_Impl =
    {
     createSfxPoolItem<XLineStartItem>, &typeid(XLineStartItem), 1, { {MID_NAME,"Name"} }
    };
#endif

/************************************************************/
extern SfxType1 aXLineEndItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType1 aXLineEndItem_Impl =
    {
     createSfxPoolItem<XLineEndItem>, &typeid(XLineEndItem), 1, { {MID_NAME,"Name"} }
    };
#endif

/************************************************************/
extern SfxType5 aSvxHyperlinkItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType5 aSvxHyperlinkItem_Impl =
    {
     createSfxPoolItem<SvxHyperlinkItem>, &typeid(SvxHyperlinkItem), 5, { {MID_HLINK_TEXT,"Text"}, {MID_HLINK_URL,"URL"}, {MID_HLINK_TARGET,"Target"}, {MID_HLINK_NAME,"Name"}, {MID_HLINK_TYPE,"Type"} }
    };
#endif

/************************************************************/
extern SfxType0 aSfxInt32Item_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxInt32Item_Impl =
    {
     createSfxPoolItem<SfxInt32Item>, &typeid(SfxInt32Item), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxPostItIdItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxPostItIdItem_Impl =
    {
     createSfxPoolItem<SvxPostItIdItem>, &typeid(SvxPostItIdItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxPostItTextItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxPostItTextItem_Impl =
    {
     createSfxPoolItem<SvxPostItTextItem>, &typeid(SvxPostItTextItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxPostItAuthorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxPostItAuthorItem_Impl =
    {
     createSfxPoolItem<SvxPostItAuthorItem>, &typeid(SvxPostItAuthorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxPostItDateItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxPostItDateItem_Impl =
    {
     createSfxPoolItem<SvxPostItDateItem>, &typeid(SvxPostItDateItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxBackgroundColorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxBackgroundColorItem_Impl =
    {
     createSfxPoolItem<SvxBackgroundColorItem>, &typeid(SvxBackgroundColorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxClipboardFormatItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxClipboardFormatItem_Impl =
    {
     createSfxPoolItem<SvxClipboardFormatItem>, &typeid(SvxClipboardFormatItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSfxTemplateItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxTemplateItem_Impl =
    {
     createSfxPoolItem<SfxTemplateItem>, &typeid(SfxTemplateItem), 0
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
extern SfxType1 aSvxTabStopItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType1 aSvxTabStopItem_Impl =
    {
     createSfxPoolItem<SvxTabStopItem>, &typeid(SvxTabStopItem), 1, { {MID_TABSTOPS,"TabStops"} }
    };
#endif

/************************************************************/
extern SfxType5 aSvxFontItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType5 aSvxFontItem_Impl =
    {
     createSfxPoolItem<SvxFontItem>, &typeid(SvxFontItem), 5, { {MID_FONT_STYLE_NAME,"StyleName"}, {MID_FONT_PITCH,"Pitch"}, {MID_FONT_CHAR_SET,"CharSet"}, {MID_FONT_FAMILY,"Family"}, {MID_FONT_FAMILY_NAME,"FamilyName"} }
    };
#endif

/************************************************************/
extern SfxType0 aSvxPostureItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxPostureItem_Impl =
    {
     createSfxPoolItem<SvxPostureItem>, &typeid(SvxPostureItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxWeightItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxWeightItem_Impl =
    {
     createSfxPoolItem<SvxWeightItem>, &typeid(SvxWeightItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxShadowedItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxShadowedItem_Impl =
    {
     createSfxPoolItem<SvxShadowedItem>, &typeid(SvxShadowedItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxContourItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxContourItem_Impl =
    {
     createSfxPoolItem<SvxContourItem>, &typeid(SvxContourItem), 0
    };
#endif

/************************************************************/
extern SfxType1 aSvxCrossedOutItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType1 aSvxCrossedOutItem_Impl =
    {
     createSfxPoolItem<SvxCrossedOutItem>, &typeid(SvxCrossedOutItem), 1, { {MID_CROSS_OUT,"Kind"} }
    };
#endif

/************************************************************/
extern SfxType3 aSvxUnderlineItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType3 aSvxUnderlineItem_Impl =
    {
     createSfxPoolItem<SvxUnderlineItem>, &typeid(SvxUnderlineItem), 3, { {MID_TL_STYLE,"LineStyle"}, {MID_TL_HASCOLOR,"HasColor"}, {MID_TL_COLOR,"Color"} }
    };
#endif

/************************************************************/
extern SfxType3 aSvxFontHeightItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType3 aSvxFontHeightItem_Impl =
    {
     createSfxPoolItem<SvxFontHeightItem>, &typeid(SvxFontHeightItem), 3, { {MID_FONTHEIGHT,"Height"}, {MID_FONTHEIGHT_PROP,"Prop"}, {MID_FONTHEIGHT_DIFF,"Diff"} }
    };
#endif

/************************************************************/
extern SfxType0 aSvxKerningItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxKerningItem_Impl =
    {
     createSfxPoolItem<SvxKerningItem>, &typeid(SvxKerningItem), 0
    };
#endif

/************************************************************/
extern SfxType2 aSvxLineSpacingItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType2 aSvxLineSpacingItem_Impl =
    {
     createSfxPoolItem<SvxLineSpacingItem>, &typeid(SvxLineSpacingItem), 2, { {MID_LINESPACE,"Mode"}, {MID_HEIGHT,"Height"} }
    };
#endif

/************************************************************/
extern SfxType5 aSvxULSpaceItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType5 aSvxULSpaceItem_Impl =
    {
     createSfxPoolItem<SvxULSpaceItem>, &typeid(SvxULSpaceItem), 5, { {MID_UP_MARGIN,"TopMargin"}, {MID_LO_MARGIN,"BottomMargin"}, {MID_CTX_MARGIN,"ContextMargin"}, {MID_UP_REL_MARGIN,"TopRelMargin"}, {MID_LO_REL_MARGIN,"BottomRelMargin"} }
    };
#endif

/************************************************************/
extern SfxType3 aSvxPageItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType3 aSvxPageItem_Impl =
    {
     createSfxPoolItem<SvxPageItem>, &typeid(SvxPageItem), 3, { {MID_PAGE_ORIENTATION,"Landscape"}, {MID_PAGE_LAYOUT,"Layout"}, {MID_PAGE_NUMTYPE,"NumType"} }
    };
#endif

/************************************************************/
extern SfxType2 aSvxLongLRSpaceItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType2 aSvxLongLRSpaceItem_Impl =
    {
     createSfxPoolItem<SvxLongLRSpaceItem>, &typeid(SvxLongLRSpaceItem), 2, { {MID_LEFT,"Left"}, {MID_RIGHT,"Right"} }
    };
#endif

/************************************************************/
extern SfxType2 aSfxPointItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType2 aSfxPointItem_Impl =
    {
     createSfxPoolItem<SfxPointItem>, &typeid(SfxPointItem), 2, { {MID_X,"X"}, {MID_Y,"Y"} }
    };
#endif

/************************************************************/
extern SfxType5 aSvxObjectItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType5 aSvxObjectItem_Impl =
    {
     createSfxPoolItem<SvxObjectItem>, &typeid(SvxObjectItem), 5, { {MID_START_X,"StartX"}, {MID_START_Y,"StartY"}, {MID_END_X,"EndX"}, {MID_END_Y,"EndY"}, {MID_LIMIT,"Limited"} }
    };
#endif

/************************************************************/
extern SfxType4 aSvxPagePosSizeItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSvxPagePosSizeItem_Impl =
    {
     createSfxPoolItem<SvxPagePosSizeItem>, &typeid(SvxPagePosSizeItem), 4, { {MID_X,"XPos"}, {MID_Y,"YPos"}, {MID_WIDTH,"Width"}, {MID_HEIGHT,"Height"} }
    };
#endif

/************************************************************/
extern SfxType4 aSfxRectangleItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSfxRectangleItem_Impl =
    {
     createSfxPoolItem<SfxRectangleItem>, &typeid(SfxRectangleItem), 4, { {MID_RECT_LEFT,"Left"}, {MID_RECT_TOP,"Top"}, {MID_WIDTH,"Width"}, {MID_HEIGHT,"Height"} }
    };
#endif

/************************************************************/
extern SfxType0 aXFillStyleItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFillStyleItem_Impl =
    {
     createSfxPoolItem<XFillStyleItem>, &typeid(XFillStyleItem), 0
    };
#endif

/************************************************************/
extern SfxType11 aXFillGradientItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType11 aXFillGradientItem_Impl =
    {
     createSfxPoolItem<XFillGradientItem>, &typeid(XFillGradientItem), 11, { {MID_NAME,"Name"}, {MID_GRADIENT_STYLE,"Style"}, {MID_GRADIENT_STARTCOLOR,"StartColor"}, {MID_GRADIENT_ENDCOLOR,"EndColor"}, {MID_GRADIENT_ANGLE,"Angle"}, {MID_GRADIENT_BORDER,"Border"}, {MID_GRADIENT_XOFFSET,"XOffset"}, {MID_GRADIENT_YOFFSET,"YOffset"}, {MID_GRADIENT_STARTINTENSITY,"StartIntensity"}, {MID_GRADIENT_ENDINTENSITY,"EndIntensity"}, {MID_GRADIENT_STEPCOUNT,"StepCount"} }
    };
#endif

/************************************************************/
extern SfxType5 aXFillHatchItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType5 aXFillHatchItem_Impl =
    {
     createSfxPoolItem<XFillHatchItem>, &typeid(XFillHatchItem), 5, { {MID_NAME,"Name"}, {MID_HATCH_STYLE,"Style"}, {MID_HATCH_COLOR,"Color"}, {MID_HATCH_DISTANCE,"Distance"}, {MID_HATCH_ANGLE,"Angle"} }
    };
#endif

/************************************************************/
extern SfxType1 aXFillBitmapItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType1 aXFillBitmapItem_Impl =
    {
     createSfxPoolItem<XFillBitmapItem>, &typeid(XFillBitmapItem), 1, { {MID_NAME,"Name"} }
    };
#endif

/************************************************************/
extern SfxType0 aXLineStyleItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXLineStyleItem_Impl =
    {
     createSfxPoolItem<XLineStyleItem>, &typeid(XLineStyleItem), 0
    };
#endif

/************************************************************/
extern SfxType7 aXLineDashItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType7 aXLineDashItem_Impl =
    {
     createSfxPoolItem<XLineDashItem>, &typeid(XLineDashItem), 7, { {MID_NAME,"Name"}, {MID_LINEDASH_STYLE,"Style"}, {MID_LINEDASH_DOTS,"Dots"}, {MID_LINEDASH_DOTLEN,"DotLen"}, {MID_LINEDASH_DASHES,"Dashes"}, {MID_LINEDASH_DASHLEN,"DashLen"}, {MID_LINEDASH_DISTANCE,"Distance"} }
    };
#endif

/************************************************************/
extern SfxType0 aSvxColorListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxColorListItem_Impl =
    {
     createSfxPoolItem<SvxColorListItem>, &typeid(SvxColorListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxGradientListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxGradientListItem_Impl =
    {
     createSfxPoolItem<SvxGradientListItem>, &typeid(SvxGradientListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxHatchListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxHatchListItem_Impl =
    {
     createSfxPoolItem<SvxHatchListItem>, &typeid(SvxHatchListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxBitmapListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxBitmapListItem_Impl =
    {
     createSfxPoolItem<SvxBitmapListItem>, &typeid(SvxBitmapListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxPatternListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxPatternListItem_Impl =
    {
     createSfxPoolItem<SvxPatternListItem>, &typeid(SvxPatternListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxDashListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxDashListItem_Impl =
    {
     createSfxPoolItem<SvxDashListItem>, &typeid(SvxDashListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxLineEndListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxLineEndListItem_Impl =
    {
     createSfxPoolItem<SvxLineEndListItem>, &typeid(SvxLineEndListItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextStyleItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextStyleItem_Impl =
    {
     createSfxPoolItem<XFormTextStyleItem>, &typeid(XFormTextStyleItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextAdjustItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextAdjustItem_Impl =
    {
     createSfxPoolItem<XFormTextAdjustItem>, &typeid(XFormTextAdjustItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextDistanceItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextDistanceItem_Impl =
    {
     createSfxPoolItem<XFormTextDistanceItem>, &typeid(XFormTextDistanceItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextStartItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextStartItem_Impl =
    {
     createSfxPoolItem<XFormTextStartItem>, &typeid(XFormTextStartItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextMirrorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextMirrorItem_Impl =
    {
     createSfxPoolItem<XFormTextMirrorItem>, &typeid(XFormTextMirrorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextOutlineItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextOutlineItem_Impl =
    {
     createSfxPoolItem<XFormTextOutlineItem>, &typeid(XFormTextOutlineItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextShadowItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextShadowItem_Impl =
    {
     createSfxPoolItem<XFormTextShadowItem>, &typeid(XFormTextShadowItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextShadowColorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextShadowColorItem_Impl =
    {
     createSfxPoolItem<XFormTextShadowColorItem>, &typeid(XFormTextShadowColorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextShadowXValItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextShadowXValItem_Impl =
    {
     createSfxPoolItem<XFormTextShadowXValItem>, &typeid(XFormTextShadowXValItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextShadowYValItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextShadowYValItem_Impl =
    {
     createSfxPoolItem<XFormTextShadowYValItem>, &typeid(XFormTextShadowYValItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXFormTextHideFormItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXFormTextHideFormItem_Impl =
    {
     createSfxPoolItem<XFormTextHideFormItem>, &typeid(XFormTextHideFormItem), 0
    };
#endif

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
extern SfxType0 aSdrOnOffItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSdrOnOffItem_Impl =
    {
     createSfxPoolItem<SdrOnOffItem>, &typeid(SdrOnOffItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSdrTextFitToSizeTypeItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSdrTextFitToSizeTypeItem_Impl =
    {
     createSfxPoolItem<SdrTextFitToSizeTypeItem>, &typeid(SdrTextFitToSizeTypeItem), 0
    };
#endif

/************************************************************/
extern SfxType3 aSvxProtectItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType3 aSvxProtectItem_Impl =
    {
     createSfxPoolItem<SvxProtectItem>, &typeid(SvxProtectItem), 3, { {MID_PROTECT_CONTENT,"Content"}, {MID_PROTECT_SIZE,"Size"}, {MID_PROTECT_POSITION,"Position"} }
    };
#endif

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
extern SfxType11 aXFillFloatTransparenceItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType11 aXFillFloatTransparenceItem_Impl =
    {
     createSfxPoolItem<XFillFloatTransparenceItem>, &typeid(XFillFloatTransparenceItem), 11, { {MID_NAME,"Name"}, {MID_GRADIENT_STYLE,"Style"}, {MID_GRADIENT_STARTCOLOR,"StartColor"}, {MID_GRADIENT_ENDCOLOR,"EndColor"}, {MID_GRADIENT_ANGLE,"Angle"}, {MID_GRADIENT_BORDER,"Border"}, {MID_GRADIENT_XOFFSET,"XOffset"}, {MID_GRADIENT_YOFFSET,"YOffset"}, {MID_GRADIENT_STARTINTENSITY,"StartIntensity"}, {MID_GRADIENT_ENDINTENSITY,"EndIntensity"}, {MID_GRADIENT_STEPCOUNT,"StepCount"} }
    };
#endif

/************************************************************/
extern SfxType0 aXLineJointItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXLineJointItem_Impl =
    {
     createSfxPoolItem<XLineJointItem>, &typeid(XLineJointItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXLineCapItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXLineCapItem_Impl =
    {
     createSfxPoolItem<XLineCapItem>, &typeid(XLineCapItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSdrPercentItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSdrPercentItem_Impl =
    {
     createSfxPoolItem<SdrPercentItem>, &typeid(SdrPercentItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aXColorItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aXColorItem_Impl =
    {
     createSfxPoolItem<XColorItem>, &typeid(XColorItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSdrMetricItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSdrMetricItem_Impl =
    {
     createSfxPoolItem<SdrMetricItem>, &typeid(SdrMetricItem), 0
    };
#endif

/************************************************************/
extern SfxType3 aSvxOverlineItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType3 aSvxOverlineItem_Impl =
    {
     createSfxPoolItem<SvxOverlineItem>, &typeid(SvxOverlineItem), 3, { {MID_TL_STYLE,"LineStyle"}, {MID_TL_HASCOLOR,"HasColor"}, {MID_TL_COLOR,"Color"} }
    };
#endif

/************************************************************/
extern SfxType0 aSvxCaseMapItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxCaseMapItem_Impl =
    {
     createSfxPoolItem<SvxCaseMapItem>, &typeid(SvxCaseMapItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aavmedia_MediaItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aavmedia_MediaItem_Impl =
    {
     createSfxPoolItem<avmedia_MediaItem>, &typeid(avmedia_MediaItem), 0
    };
#endif

/************************************************************/
extern SfxType8 aSvxBoxItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType8 aSvxBoxItem_Impl =
    {
     createSfxPoolItem<SvxBoxItem>, &typeid(SvxBoxItem), 8, { {MID_LEFT_BORDER,"LeftBorder"}, {LEFT_BORDER_DISTANCE,"LeftDistance"}, {MID_RIGHT_BORDER,"RightBorder"}, {RIGHT_BORDER_DISTANCE,"RightDistance"}, {MID_TOP_BORDER,"TopBorder"}, {TOP_BORDER_DISTANCE,"TopDistance"}, {MID_BOTTOM_BORDER,"BottomBorder"}, {BOTTOM_BORDER_DISTANCE,"BottomDistance"} }
    };
#endif

/************************************************************/
extern SfxType5 aSvxBoxInfoItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType5 aSvxBoxInfoItem_Impl =
    {
     createSfxPoolItem<SvxBoxInfoItem>, &typeid(SvxBoxInfoItem), 5, { {MID_HORIZONTAL,"Horizontal"}, {MID_VERTICAL,"Vertical"}, {MID_FLAGS,"Flags"}, {MID_VALIDFLAGS,"ValidFlags"}, {MID_DISTANCE,"DefaultDistance"} }
    };
#endif

/************************************************************/
extern SfxType4 aSvxLineItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSvxLineItem_Impl =
    {
     createSfxPoolItem<SvxLineItem>, &typeid(SvxLineItem), 4, { {MID_FG_COLOR,"LineFGColor"}, {MID_OUTER_WIDTH,"LineOutWidth"}, {MID_INNER_WIDTH,"LineInWidth"}, {MID_DISTANCE,"LineDistance"} }
    };
#endif

/************************************************************/
#ifdef ShellClass_StarImpress
#undef ShellClass
#undef ShellClass_StarImpress
#define ShellClass StarImpress
#endif

/************************************************************/
#ifdef ShellClass_SdModule
#undef ShellClass
#undef ShellClass_SdModule
#define ShellClass SdModule
static SfxFormalArgument aSdModuleArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "Region", SID_TEMPLATE_REGIONNAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_TEMPLATE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "URL", SID_FILE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", SID_FILTER_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "OpenFlags", SID_OPTIONS },
     { (const SfxType*) &aSfxStringItem_Impl, "Password", SID_PASSWORD },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterOptions", SID_FILE_FILTEROPTIONS },
     { (const SfxType*) &aSfxInt16Item_Impl, "Version", SID_VERSION },
     { (const SfxType*) &aSfxStringItem_Impl, "Referer", SID_REFERER },
     { (const SfxType*) &aSfxStringItem_Impl, "SuggestedSaveAsDir", SID_DEFAULTFILEPATH },
     { (const SfxType*) &aSfxStringItem_Impl, "SuggestedSaveAsName", SID_DEFAULTFILENAME },
     { (const SfxType*) &aSfxStringItem_Impl, "URL", SID_FILE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", SID_FILTER_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "OpenFlags", SID_OPTIONS },
     { (const SfxType*) &aSfxStringItem_Impl, "Password", SID_PASSWORD },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterOptions", SID_FILE_FILTEROPTIONS },
     { (const SfxType*) &aSfxInt16Item_Impl, "Version", SID_VERSION },
     { (const SfxType*) &aSfxStringItem_Impl, "Referer", SID_REFERER },
     { (const SfxType*) &aSfxUnoFrameItem_Impl, "Frame", SID_FILLFRAME },
     { (const SfxType*) &aSfxBoolItem_Impl, "Hidden", SID_HIDDEN },
     { (const SfxType*) &aSfxLockBytesItem_Impl, "RtfOutline", SID_OUTLINE_TO_IMPRESS }
    };

SFX_EXEC_STUB(SdModule,Execute)
SFX_STATE_STUB(SdModule,GetState)

static SfxSlot aSdModuleSlots_Impl[] =
    {
     // Slot Nr. 0 : 5500
     SFX_NEW_SLOT_ARG( SdModule,SID_NEWDOC,SfxGroupId::Application,
                       &aSdModuleSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewDoc" ),
     // Slot Nr. 1 : 5501
     SFX_NEW_SLOT_ARG( SdModule,SID_OPENDOC,SfxGroupId::Application,
                       &aSdModuleSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 9/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Open" ),
     // Slot Nr. 2 : 6676
     SFX_NEW_SLOT_ARG( SdModule,SID_OPENHYPERLINK,SfxGroupId::Application,
                       &aSdModuleSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       11/*Offset*/, 7/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenHyperlink" ),
     // Slot Nr. 3 : 6686
     SFX_NEW_SLOT_ARG( SdModule,SID_NEWSD,SfxGroupId::Application,
                       &aSdModuleSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       18/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewPresentation" ),
     // Slot Nr. 4 : 10443
     SFX_NEW_SLOT_ARG( SdModule,SID_OUTLINE_TO_IMPRESS,SfxGroupId::Special,
                       &aSdModuleSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       20/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SendOutlineToImpress" ),
     // Slot Nr. 5 : 10889
     SFX_NEW_SLOT_ARG( SdModule,SID_ATTR_CHAR_CJK_LANGUAGE,SfxGroupId::Edit,
                       &aSdModuleSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLanguageItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"DocumentLanguageCJK" ),
     // Slot Nr. 6 : 10894
     SFX_NEW_SLOT_ARG( SdModule,SID_ATTR_CHAR_CTL_LANGUAGE,SfxGroupId::Edit,
                       &aSdModuleSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLanguageItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"DocumentLanguageCTL" ),
     // Slot Nr. 7 : 12007
     SFX_NEW_SLOT_ARG( SdModule,SID_ATTR_LANGUAGE,SfxGroupId::Edit,
                       &aSdModuleSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLanguageItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"DocumentLanguage" ),
     // Slot Nr. 8 : 12008
     SFX_NEW_SLOT_ARG( SdModule,SID_ATTR_METRIC,SfxGroupId::NONE,
                       &aSdModuleSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"MetricUnit" ),
     // Slot Nr. 9 : 12021
     SFX_NEW_SLOT_ARG( SdModule,SID_AUTOSPELL_CHECK,SfxGroupId::Options,
                       &aSdModuleSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(SdModule,Execute),SFX_STUB_PTR(SdModule,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"SpellOnline" )
    };
#endif

/************************************************************/
#ifdef ShellClass_DrawView
#undef ShellClass
#undef ShellClass_DrawView
#define ShellClass DrawView
#endif

/************************************************************/
#ifdef ShellClass_GraphicEditView
#undef ShellClass
#undef ShellClass_GraphicEditView
#define ShellClass GraphicEditView
#endif

/************************************************************/
#ifdef ShellClass_GraphicViewShell
#undef ShellClass
#undef ShellClass_GraphicViewShell
#define ShellClass GraphicViewShell
static SfxFormalArgument aGraphicViewShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxUInt32Item_Impl, "Format", SID_PASTE_SPECIAL },
     { (const SfxType*) &aSfxUInt32Item_Impl, "SelectedFormat", SID_CLIPBOARD_FORMAT_ITEMS },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_NEW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_EDIT },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_DELETE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Template", SID_STYLE_APPLY },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "FamilyName", SID_STYLE_FAMILYNAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Style", SID_APPLY_STYLE },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_NEW_BY_EXAMPLE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_UPDATE_BY_EXAMPLE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxGlobalNameItem_Impl, "ClassId", SID_INSERT_OBJECT },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "URL", FN_PARAM_2 },
     { (const SfxType*) &aSvxSizeItem_Impl, "Margin", FN_PARAM_3 },
     { (const SfxType*) &aSfxByteItem_Impl, "ScrollingMode", FN_PARAM_4 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsBorder", FN_PARAM_5 },
     { (const SfxType*) &aSfxStringItem_Impl, "Bookmark", SID_JUMPTOMARK },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_CLASSIFICATION_APPLY },
     { (const SfxType*) &aSfxStringItem_Impl, "Type", SID_TYPE_NAME },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "PersistentCopy", SID_FORMATPAINTBRUSH },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_HIDE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_SHOW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxBoolItem_Impl, "CreateDirectly", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxColorItem_Impl, "Color", SID_ATTR_CHAR_COLOR },
     { (const SfxType*) &aSvxLRSpaceItem_Impl, "LRSpace", SID_ATTR_PARA_LRSPACE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformPosX", SID_ATTR_TRANSFORM_POS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformPosY", SID_ATTR_TRANSFORM_POS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformWidth", SID_ATTR_TRANSFORM_WIDTH },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformHeight", SID_ATTR_TRANSFORM_HEIGHT },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationDeltaAngle", SID_ATTR_TRANSFORM_DELTA_ANGLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationAngle", SID_ATTR_TRANSFORM_ANGLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationX", SID_ATTR_TRANSFORM_ROT_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationY", SID_ATTR_TRANSFORM_ROT_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt16Item_Impl, "FillTransparence", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "FillColor", FN_PARAM_2 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "LineStyle", FN_PARAM_3 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsSticky", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "ShapeName", SID_SHAPE_NAME },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxBoolItem_Impl, "ColHeaders", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "RowHeaders", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "InNewTable", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "RangeList", FN_PARAM_5 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Columns", SID_ATTR_TABLE_COLUMN },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Rows", SID_ATTR_TABLE_ROW },
     { (const SfxType*) &aSfxStringItem_Impl, "Page", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aXFillColorItem_Impl, "FillColor", SID_ATTR_FILL_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "FillGradientJSON", SID_FILL_GRADIENT_JSON },
     { (const SfxType*) &aSvxDoubleItem_Impl, "Width", SID_ATTR_LINE_WIDTH_ARG },
     { (const SfxType*) &aXLineWidthItem_Impl, "LineWidth", SID_ATTR_LINE_WIDTH },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aXLineColorItem_Impl, "XLineColor", SID_ATTR_LINE_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", SID_INSERT_GRAPHIC },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", FN_PARAM_FILTER },
     { (const SfxType*) &aSfxBoolItem_Impl, "AsLink", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Style", FN_PARAM_2 },
     { (const SfxType*) &aSvxGalleryItem_Impl, "GalleryItem", SID_GALLERY_FORMATS },
     { (const SfxType*) &aSvxLongULSpaceItem_Impl, "Space", SID_ATTR_LONG_ULSPACE },
     { (const SfxType*) &aSfxStringItem_Impl, "Page", FN_PARAM_1 },
     { (const SfxType*) &aXLineStartItem_Impl, "LineStart", SID_ATTR_LINE_START },
     { (const SfxType*) &aXLineEndItem_Impl, "LineEnd", SID_ATTR_LINE_END },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartWidth", SID_ATTR_LINE_STARTWIDTH },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndWidth", SID_ATTR_LINE_ENDWIDTH },
     { (const SfxType*) &aSvxHyperlinkItem_Impl, "Hyperlink", SID_HYPERLINK_SETLINK },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterX", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "CenterY", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisX", ID_VAL_AXIS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "AxisY", ID_VAL_AXIS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartAngle", ID_VAL_ANGLESTART },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndAngle", ID_VAL_ANGLEEND },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Transparence", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", FN_PARAM_2 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Width", FN_PARAM_3 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsSticky", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "ShapeName", SID_SHAPE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbols", SID_CHARMAP },
     { (const SfxType*) &aSfxStringItem_Impl, "FontName", SID_ATTR_SPECIALCHAR },
     { (const SfxType*) &aSvxDoubleItem_Impl, "Depth", SID_EXTRUSION_DEPTH },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Metric", SID_ATTR_METRIC },
     { (const SfxType*) &aSfxStringItem_Impl, "FontworkShapeType", SID_FONTWORK_SHAPE_TYPE },
     { (const SfxType*) &aSfxStringItem_Impl, "BasicShapes", SID_DRAWTBX_CS_BASIC },
     { (const SfxType*) &aSfxStringItem_Impl, "SymbolShapes", SID_DRAWTBX_CS_SYMBOL },
     { (const SfxType*) &aSfxStringItem_Impl, "ArrowShapes", SID_DRAWTBX_CS_ARROW },
     { (const SfxType*) &aSfxStringItem_Impl, "FlowChartShapes", SID_DRAWTBX_CS_FLOWCHART },
     { (const SfxType*) &aSfxStringItem_Impl, "CalloutShapes", SID_DRAWTBX_CS_CALLOUT },
     { (const SfxType*) &aSfxStringItem_Impl, "StarShapes", SID_DRAWTBX_CS_STAR },
     { (const SfxType*) &aSfxInt32Item_Impl, "FontworkCharacterSpacing", SID_FONTWORK_CHARACTER_SPACING },
     { (const SfxType*) &aSfxBoolItem_Impl, "MirrorVert", ID_VAL_MIRRORVERT },
     { (const SfxType*) &aSvxPostItIdItem_Impl, "Id", SID_ATTR_POSTIT_ID },
     { (const SfxType*) &aSvxPostItIdItem_Impl, "Id", SID_ATTR_POSTIT_ID },
     { (const SfxType*) &aSvxPostItTextItem_Impl, "Text", SID_ATTR_POSTIT_TEXT },
     { (const SfxType*) &aSvxPostItAuthorItem_Impl, "Author", SID_ATTR_POSTIT_AUTHOR },
     { (const SfxType*) &aSvxPostItDateItem_Impl, "Date", SID_ATTR_POSTIT_DATE },
     { (const SfxType*) &aSvxPostItTextItem_Impl, "Text", SID_ATTR_POSTIT_TEXT },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxBackgroundColorItem_Impl, "CharBackColor", SID_ATTR_CHAR_BACK_COLOR },
     { (const SfxType*) &aSfxBoolItem_Impl, "On", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "On", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "PageName", ID_VAL_PAGENAME },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatLayout", ID_VAL_WHATLAYOUT },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPageBack", ID_VAL_ISPAGEBACK },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPageObj", ID_VAL_ISPAGEOBJ },
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", ID_VAL_DUMMY1 },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", ID_VAL_DUMMY2 },
     { (const SfxType*) &aSfxStringItem_Impl, "LayerName", ID_VAL_LAYERNAME },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsVisible", ID_VAL_ISVISIBLE },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsLocked", ID_VAL_ISLOCKED },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPrintable", ID_VAL_ISPRINTABLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatPage", ID_VAL_WHATPAGE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatKind", ID_VAL_WHATKIND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatLayer", ID_VAL_WHATLAYER },
     { (const SfxType*) &aSfxStringItem_Impl, "LayerName", ID_VAL_LAYERNAME },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsVisible", ID_VAL_ISVISIBLE },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsLocked", ID_VAL_ISLOCKED },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPrintable", ID_VAL_ISPRINTABLE },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsActive", ID_VAL_ISACTIVE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatKind", ID_VAL_WHATKIND },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsActive", ID_VAL_ISACTIVE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatLayer", ID_VAL_WHATLAYER },
     { (const SfxType*) &aSfxBoolItem_Impl, "Isactive", ID_VAL_ISACTIVE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "PageWidth", ID_VAL_PAGEWIDTH },
     { (const SfxType*) &aSfxUInt32Item_Impl, "PageHeight", ID_VAL_PAGEHEIGHT },
     { (const SfxType*) &aSfxBoolItem_Impl, "ScaleObjects", ID_VAL_SCALEOBJECTS },
     { (const SfxType*) &aSfxUInt32Item_Impl, "PageLeft", ID_VAL_PAGELEFT },
     { (const SfxType*) &aSfxUInt32Item_Impl, "PageRight", ID_VAL_PAGERIGHT },
     { (const SfxType*) &aSfxUInt32Item_Impl, "PageTop", ID_VAL_PAGETOP },
     { (const SfxType*) &aSfxUInt32Item_Impl, "PageBottom", ID_VAL_PAGEBOTTOM },
     { (const SfxType*) &aSfxBoolItem_Impl, "ScaleObjects", ID_VAL_SCALEOBJECTS },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatKind", ID_VAL_WHATKIND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatKind", ID_VAL_WHATKIND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatKind", ID_VAL_WHATKIND },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Style", ID_VAL_STYLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Red", ID_VAL_RED },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Green", ID_VAL_GREEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Blue", ID_VAL_BLUE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Style", ID_VAL_STYLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Angle", ID_VAL_ANGLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Border", ID_VAL_BORDER },
     { (const SfxType*) &aSfxUInt32Item_Impl, "XOffset", ID_VAL_CENTER_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "YOffset", ID_VAL_CENTER_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartIntens", ID_VAL_STARTINTENS },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndIntens", ID_VAL_ENDINTENS },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Style", ID_VAL_STYLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Width", ID_VAL_WIDTH },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Style", ID_VAL_STYLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Dots", ID_VAL_DOTS },
     { (const SfxType*) &aSfxUInt32Item_Impl, "DotLen", ID_VAL_DOTLEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Dashes", ID_VAL_DASHES },
     { (const SfxType*) &aSfxUInt32Item_Impl, "DashLen", ID_VAL_DASHLEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Distance", ID_VAL_DISTANCE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_RENAMEPAGE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Style", ID_VAL_STYLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Distance", ID_VAL_DISTANCE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Angle", ID_VAL_ANGLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Red", ID_VAL_RED },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Green", ID_VAL_GREEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Blue", ID_VAL_BLUE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Red", ID_VAL_RED },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Green", ID_VAL_GREEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Blue", ID_VAL_BLUE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Red", ID_VAL_RED },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Green", ID_VAL_GREEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Blue", ID_VAL_BLUE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Red", ID_VAL_RED },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Green", ID_VAL_GREEN },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Blue", ID_VAL_BLUE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", ID_VAL_INDEX },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsAutoGrow", ID_VAL_DUMMY0 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsCenter", ID_VAL_DUMMY1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsFitToSize", ID_VAL_DUMMY2 },
     { (const SfxType*) &aSfxUInt32Item_Impl, "LeftBorder", ID_VAL_DUMMY3 },
     { (const SfxType*) &aSfxUInt32Item_Impl, "RightBorder", ID_VAL_DUMMY4 },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TopBorder", ID_VAL_DUMMY5 },
     { (const SfxType*) &aSfxUInt32Item_Impl, "BottomBorder", ID_VAL_DUMMY6 },
     { (const SfxType*) &aSfxStringItem_Impl, "PageName", ID_VAL_PAGENAME },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatLayout", ID_VAL_WHATLAYOUT },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPageBack", ID_VAL_ISPAGEBACK },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPageObj", ID_VAL_ISPAGEOBJ },
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", SID_SELECT_BACKGROUND },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", FN_PARAM_FILTER },
     { (const SfxType*) &aSfxBoolItem_Impl, "AsLink", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Style", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "PageName", ID_VAL_PAGENAME },
     { (const SfxType*) &aSfxUInt32Item_Impl, "WhatLayout", ID_VAL_WHATLAYOUT },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPageBack", ID_VAL_ISPAGEBACK },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsPageObj", ID_VAL_ISPAGEOBJ }
    };

SFX_EXEC_STUB(GraphicViewShell,FuTemporary)
SFX_STATE_STUB(GraphicViewShell,GetMenuState)
SFX_EXEC_STUB(GraphicViewShell,FuSupport)
SFX_EXEC_STUB(GraphicViewShell,ExecCtrl)
SFX_STATE_STUB(GraphicViewShell,GetCtrlState)
SFX_STATE_STUB(GraphicViewShell,GetAttrState)
SFX_EXEC_STUB(GraphicViewShell,FuPermanent)
SFX_STATE_STUB(GraphicViewShell,GetStatusBarState)
SFX_EXEC_STUB(GraphicViewShell,ExecRuler)
SFX_STATE_STUB(GraphicViewShell,GetRulerState)
SFX_EXEC_STUB(GraphicViewShell,ExecChar)
SFX_STATE_STUB(GraphicViewShell,GetPageProperties)
SFX_EXEC_STUB(GraphicViewShell,SetPageProperties)
SFX_STATE_STUB(GraphicViewShell,GetMarginProperties)
SFX_STATE_STUB(GraphicViewShell,GetDrawAttrState)
SFX_EXEC_STUB(GraphicViewShell,FuTable)
SFX_STATE_STUB(GraphicViewShell,GetTableMenuState)
SFX_EXEC_STUB(GraphicViewShell,ExecStatusBar)
SFX_EXEC_STUB(GraphicViewShell,Execute)
SFX_EXEC_STUB(GraphicViewShell,ExecFormText)
SFX_STATE_STUB(GraphicViewShell,GetFormTextState)
SFX_EXEC_STUB(GraphicViewShell,ExecGallery)
SFX_STATE_STUB(GraphicViewShell,GetState)
SFX_EXEC_STUB(GraphicViewShell,ExecBmpMask)
SFX_STATE_STUB(GraphicViewShell,GetBmpMaskState)
SFX_EXEC_STUB(GraphicViewShell,ExecIMap)
SFX_STATE_STUB(GraphicViewShell,GetIMapState)
SFX_EXEC_STUB(GraphicViewShell,ExecEffectWin)
SFX_EXEC_STUB(GraphicViewShell,ExecutePropPanelAttr)
SFX_STATE_STUB(GraphicViewShell,GetStatePropPanelAttr)
SFX_EXEC_STUB(GraphicViewShell,ExecuteAnnotation)
SFX_STATE_STUB(GraphicViewShell,GetAnnotationState)
SFX_EXEC_STUB(GraphicViewShell,FuSupportRotate)
SFX_EXEC_STUB(GraphicViewShell,ExecGoToNextPage)
SFX_STATE_STUB(GraphicViewShell,GetStateGoToNextPage)
SFX_EXEC_STUB(GraphicViewShell,ExecGoToPreviousPage)
SFX_STATE_STUB(GraphicViewShell,GetStateGoToPreviousPage)
SFX_EXEC_STUB(GraphicViewShell,ExecGoToFirstPage)
SFX_STATE_STUB(GraphicViewShell,GetStateGoToFirstPage)
SFX_EXEC_STUB(GraphicViewShell,ExecGoToLastPage)
SFX_STATE_STUB(GraphicViewShell,GetStateGoToLastPage)
SFX_STATE_STUB(GraphicViewShell,GetSnapItemState)
SFX_EXEC_STUB(GraphicViewShell,ExecOptionsBar)
SFX_STATE_STUB(GraphicViewShell,GetOptionsBarState)
SFX_EXEC_STUB(GraphicViewShell,AttrExec)
SFX_STATE_STUB(GraphicViewShell,AttrState)
SFX_EXEC_STUB(GraphicViewShell,ExecNavigatorWin)
SFX_STATE_STUB(GraphicViewShell,GetNavigatorWinState)

static SfxSlot aGraphicViewShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5310
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONTEXT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"Context" ),
     // Slot Nr. 1 : 5311
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PASTE_SPECIAL,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteSpecial" ),
     // Slot Nr. 2 : 5312
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CLIPBOARD_FORMAT_ITEMS,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxClipboardFormatItem,
                       1/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClipboardFormatItems" ),
     // Slot Nr. 3 : 5314
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PASTE_UNFORMATTED,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteUnformatted" ),
     // Slot Nr. 4 : 5405
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EMOJI_CONTROL,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"EmojiControl" ),
     // Slot Nr. 5 : 5406
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHARMAP_CONTROL,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"CharmapControl" ),
     // Slot Nr. 6 : 5508
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RELOAD,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Reload" ),
     // Slot Nr. 7 : 5542
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_FAMILY2,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"ParaStyle" ),
     // Slot Nr. 8 : 5543
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_FAMILY3,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"FrameStyle" ),
     // Slot Nr. 9 : 5549
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_NEW,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewStyle" ),
     // Slot Nr. 10 : 5550
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_EDIT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditStyle" ),
     // Slot Nr. 11 : 5551
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_DELETE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteStyle" ),
     // Slot Nr. 12 : 5552
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_APPLY,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       8/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleApply" ),
     // Slot Nr. 13 : 5554
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_WATERCAN,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       12/*Offset*/, 0, SfxSlotMode::NONE,"StyleWatercanMode" ),
     // Slot Nr. 14 : 5555
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_NEW_BY_EXAMPLE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       12/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleNewByExample" ),
     // Slot Nr. 15 : 5556
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_UPDATE_BY_EXAMPLE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleUpdateByExample" ),
     // Slot Nr. 16 : 5561
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_OBJECT,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObject" ),
     // Slot Nr. 17 : 5563
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLOATINGFRAME,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 5/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObjectFloatingFrame" ),
     // Slot Nr. 18 : 5565
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_DRAGHIERARCHIE,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DragHierarchy" ),
     // Slot Nr. 19 : 5598
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_JUMPTOMARK,SfxGroupId::Navigator,
                       &aGraphicViewShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToMark" ),
     // Slot Nr. 20 : 5672
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CLASSIFICATION_APPLY,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       23/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClassificationApply" ),
     // Slot Nr. 21 : 5679
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_POLY_MERGE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Merge" ),
     // Slot Nr. 22 : 5680
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_POLY_SUBSTRACT,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Substract" ),
     // Slot Nr. 23 : 5681
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_POLY_INTERSECT,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Intersect" ),
     // Slot Nr. 24 : 5683
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DISTRIBUTE_DLG,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DistributeSelection" ),
     // Slot Nr. 25 : 5684
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EQUALIZEWIDTH,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EqualizeWidth" ),
     // Slot Nr. 26 : 5685
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EQUALIZEHEIGHT,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EqualizeHeight" ),
     // Slot Nr. 27 : 5700
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_REDO,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       25/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 28 : 5701
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_UNDO,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       27/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 29 : 5710
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CUT,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 30 : 5711
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COPY,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 31 : 5712
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PASTE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 32 : 5713
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 33 : 5715
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMATPAINTBRUSH,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       31/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatPaintbrush" ),
     // Slot Nr. 34 : 5723
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SELECTALL,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 35 : 5792
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_UNICODE_NOTATION_TOGGLE,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UnicodeNotationToggle" ),
     // Slot Nr. 36 : 5960
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GALLERY,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[149] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"Gallery" ),
     // Slot Nr. 37 : 6000
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECTRESIZE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InPlaceObjectResize" ),
     // Slot Nr. 38 : 6588
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MAIL_SCROLLBODY_PAGEDOWN,SfxGroupId::Explorer,
                       &aGraphicViewShellSlots_Impl[170] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ScrollBodyPageDown" ),
     // Slot Nr. 39 : 6603
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_HIDE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideStyle" ),
     // Slot Nr. 40 : 6604
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STYLE_SHOW,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       34/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowStyle" ),
     // Slot Nr. 41 : 6694
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_AVMEDIA_PLAYER,SfxGroupId::Application,
                       &aGraphicViewShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AVMediaPlayer" ),
     // Slot Nr. 42 : 6696
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_AVMEDIA,SfxGroupId::Application,
                       &aGraphicViewShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertAVMedia" ),
     // Slot Nr. 43 : 10000
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_ZOOM,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[129] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetStatusBarState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"Zoom" ),
     // Slot Nr. 44 : 10002
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TABSTOP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxTabStopItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"Tabstops" ),
     // Slot Nr. 45 : 10006
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Text" ),
     // Slot Nr. 46 : 10007
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_FONT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"CharFontName" ),
     // Slot Nr. 47 : 10008
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_POSTURE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPostureItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"Italic" ),
     // Slot Nr. 48 : 10009
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_WEIGHT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxWeightItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"Bold" ),
     // Slot Nr. 49 : 10010
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_SHADOWED,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxShadowedItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"Shadowed" ),
     // Slot Nr. 50 : 10012
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_CONTOUR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxContourItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"OutlineFont" ),
     // Slot Nr. 51 : 10013
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_STRIKEOUT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCrossedOutItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"Strikeout" ),
     // Slot Nr. 52 : 10014
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_UNDERLINE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxUnderlineItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"Underline" ),
     // Slot Nr. 53 : 10015
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_FONTHEIGHT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontHeightItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"FontHeight" ),
     // Slot Nr. 54 : 10017
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_COLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       37/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Color" ),
     // Slot Nr. 55 : 10018
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_KERNING,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxKerningItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"Spacing" ),
     // Slot Nr. 56 : 10028
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_ADJUST_LEFT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"LeftPara" ),
     // Slot Nr. 57 : 10029
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_ADJUST_RIGHT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"RightPara" ),
     // Slot Nr. 58 : 10030
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_ADJUST_CENTER,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"CenterPara" ),
     // Slot Nr. 59 : 10031
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_ADJUST_BLOCK,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"JustifyPara" ),
     // Slot Nr. 60 : 10033
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_LINESPACE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineSpacingItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"LineSpacing" ),
     // Slot Nr. 61 : 10042
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_ULSPACE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxULSpaceItem,
                       39/*Offset*/, 0, SfxSlotMode::NONE,"ULSpacing" ),
     // Slot Nr. 62 : 10043
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PARA_LRSPACE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[110] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLRSpaceItem,
                       39/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LeftRightParaMargin" ),
     // Slot Nr. 63 : 10050
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPageItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"AttributePage" ),
     // Slot Nr. 64 : 10051
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_SIZE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[318] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSizeItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"AttributePageSize" ),
     // Slot Nr. 65 : 10062
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_LRSPACE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetMarginProperties),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLongLRSpaceItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"PageLRMargin" ),
     // Slot Nr. 66 : 10063
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_ULSPACE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetMarginProperties),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLongULSpaceItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"PageULMargin" ),
     // Slot Nr. 67 : 10079
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER_NULL_OFFSET,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxPointItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"RulerNullOffset" ),
     // Slot Nr. 68 : 10081
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER_OBJECT,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxObjectItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"RulerObject" ),
     // Slot Nr. 69 : 10082
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER_PAGE_POS,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPagePosSizeItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"RulerPagePos" ),
     // Slot Nr. 70 : 10084
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER_LR_MIN_MAX,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[151] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxRectangleItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"HeaderFooterBorder" ),
     // Slot Nr. 71 : 10087
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 8/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TransformDialog" ),
     // Slot Nr. 72 : 10088
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_POS_X,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformPosX" ),
     // Slot Nr. 73 : 10089
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_POS_Y,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformPosY" ),
     // Slot Nr. 74 : 10090
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_WIDTH,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformWidth" ),
     // Slot Nr. 75 : 10091
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_HEIGHT,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformHeight" ),
     // Slot Nr. 76 : 10093
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_ROT_X,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformRotationX" ),
     // Slot Nr. 77 : 10094
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_ROT_Y,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformRotationY" ),
     // Slot Nr. 78 : 10095
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_ANGLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[131] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"TransformRotationAngle" ),
     // Slot Nr. 79 : 10096
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_TOOLBOX,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"ZoomToolBox" ),
     // Slot Nr. 80 : 10097
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_OUT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPlus" ),
     // Slot Nr. 81 : 10098
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_IN,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomMinus" ),
     // Slot Nr. 82 : 10099
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SIZE_REAL,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Zoom100Percent" ),
     // Slot Nr. 83 : 10100
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SIZE_PAGE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPage" ),
     // Slot Nr. 84 : 10101
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SIZE_ALL,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomOptimal" ),
     // Slot Nr. 85 : 10102
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_LINE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"Line" ),
     // Slot Nr. 86 : 10103
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_XLINE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       48/*Offset*/, 0, SfxSlotMode::NONE,"Line_Diagonal" ),
     // Slot Nr. 87 : 10104
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_RECT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       48/*Offset*/, 9/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Rect" ),
     // Slot Nr. 88 : 10105
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_RECT_ROUND,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       57/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Rect_Rounded" ),
     // Slot Nr. 89 : 10110
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_ELLIPSE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       61/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Ellipse" ),
     // Slot Nr. 90 : 10112
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_PIE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Pie" ),
     // Slot Nr. 91 : 10114
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_ARC,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       71/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Arc" ),
     // Slot Nr. 92 : 10115
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLECUT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       77/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CircleCut" ),
     // Slot Nr. 93 : 10117
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_POLYGON,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       83/*Offset*/, 0, SfxSlotMode::NONE,"Polygon" ),
     // Slot Nr. 94 : 10118
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_BEZIER_FILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       83/*Offset*/, 0, SfxSlotMode::NONE,"BezierFill" ),
     // Slot Nr. 95 : 10126
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BEZIER_EDIT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleObjectBezierMode" ),
     // Slot Nr. 96 : 10128
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_SELECT,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       83/*Offset*/, 0, SfxSlotMode::NONE,"SelectObject" ),
     // Slot Nr. 97 : 10129
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ROTATE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleObjectRotateMode" ),
     // Slot Nr. 98 : 10130
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0, SfxSlotMode::NONE,"ObjectAlign" ),
     // Slot Nr. 99 : 10131
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN_LEFT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectAlignLeft" ),
     // Slot Nr. 100 : 10132
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN_CENTER,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[101] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignCenter" ),
     // Slot Nr. 101 : 10133
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN_RIGHT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[102] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectAlignRight" ),
     // Slot Nr. 102 : 10134
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN_UP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[103] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignUp" ),
     // Slot Nr. 103 : 10135
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN_MIDDLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[104] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignMiddle" ),
     // Slot Nr. 104 : 10136
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_ALIGN_DOWN,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[105] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignDown" ),
     // Slot Nr. 105 : 10140
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_DIAGRAM,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[107] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObjectChart" ),
     // Slot Nr. 106 : 10141
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_TABLE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[204] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTable),SFX_STUB_PTR(GraphicViewShell,GetTableMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       87/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertTable" ),
     // Slot Nr. 107 : 10142
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTRIBUTES_AREA,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[108] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       89/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatArea" ),
     // Slot Nr. 108 : 10143
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTRIBUTES_LINE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[109] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       89/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatLine" ),
     // Slot Nr. 109 : 10156
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OUTLINE_BULLET,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[128] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       89/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineBullet" ),
     // Slot Nr. 110 : 10164
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_STYLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[111] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillStyleItem,
                       90/*Offset*/, 0, SfxSlotMode::NONE,"FillStyle" ),
     // Slot Nr. 111 : 10165
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_COLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[112] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillColorItem,
                       90/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillColor" ),
     // Slot Nr. 112 : 10166
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_GRADIENT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[113] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillGradientItem,
                       92/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillGradient" ),
     // Slot Nr. 113 : 10167
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_HATCH,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[114] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillHatchItem,
                       93/*Offset*/, 0, SfxSlotMode::NONE,"FillHatch" ),
     // Slot Nr. 114 : 10168
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_BITMAP,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[115] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillBitmapItem,
                       93/*Offset*/, 0, SfxSlotMode::NONE,"FillBitmap" ),
     // Slot Nr. 115 : 10169
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_STYLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[116] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineStyleItem,
                       93/*Offset*/, 0, SfxSlotMode::NONE,"XLineStyle" ),
     // Slot Nr. 116 : 10170
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_DASH,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[117] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineDashItem,
                       93/*Offset*/, 0, SfxSlotMode::NONE,"LineDash" ),
     // Slot Nr. 117 : 10171
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_WIDTH,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[118] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineWidthItem,
                       93/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineWidth" ),
     // Slot Nr. 118 : 10172
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_COLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[119] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineColorItem,
                       95/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"XLineColor" ),
     // Slot Nr. 119 : 10173
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_START,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[120] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineStartItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"LineStart" ),
     // Slot Nr. 120 : 10174
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_END,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[121] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineEndItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"LineEnd" ),
     // Slot Nr. 121 : 10179
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COLOR_TABLE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[122] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"ColorTableState" ),
     // Slot Nr. 122 : 10180
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GRADIENT_LIST,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[123] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxGradientListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"GradientListState" ),
     // Slot Nr. 123 : 10181
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HATCH_LIST,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[124] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHatchListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"HatchListState" ),
     // Slot Nr. 124 : 10182
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BITMAP_LIST,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[125] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBitmapListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"BitmapListState" ),
     // Slot Nr. 125 : 10183
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PATTERN_LIST,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[126] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPatternListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"PatternListState" ),
     // Slot Nr. 126 : 10184
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DASH_LIST,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[127] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxDashListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"DashListState" ),
     // Slot Nr. 127 : 10185
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINEEND_LIST,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[156] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineEndListItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"LineEndListState" ),
     // Slot Nr. 128 : 10217
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TABLE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[133] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       97/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSpreadsheet" ),
     // Slot Nr. 129 : 10223
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_POSITION,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[130] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecStatusBar),SFX_STUB_PTR(GraphicViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxPointItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"Position" ),
     // Slot Nr. 130 : 10224
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_SIZE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[280] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecStatusBar),SFX_STUB_PTR(GraphicViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSizeItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"Size" ),
     // Slot Nr. 131 : 10236
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_PROTECT_POS,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[132] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"ProtectPos" ),
     // Slot Nr. 132 : 10237
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_PROTECT_SIZE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[162] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       97/*Offset*/, 0, SfxSlotMode::NONE,"ProtectSize" ),
     // Slot Nr. 133 : 10241
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_GRAPHIC,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[134] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       97/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertGraphic" ),
     // Slot Nr. 134 : 10243
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SPELL_DIALOG,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[135] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,Execute),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"SpellDialog" ),
     // Slot Nr. 135 : 10245
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_THESAURUS,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[136] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusDialog" ),
     // Slot Nr. 136 : 10254
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CAPTION,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[137] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"DrawCaption" ),
     // Slot Nr. 137 : 10256
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[153] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWork" ),
     // Slot Nr. 138 : 10257
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_STYLE,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[139] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextStyleItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextStyle" ),
     // Slot Nr. 139 : 10258
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_ADJUST,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[140] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextAdjustItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextAdjust" ),
     // Slot Nr. 140 : 10259
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_DISTANCE,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[141] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextDistanceItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextDistance" ),
     // Slot Nr. 141 : 10260
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_START,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[142] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextStartItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextStart" ),
     // Slot Nr. 142 : 10261
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_MIRROR,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[143] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextMirrorItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextMirror" ),
     // Slot Nr. 143 : 10262
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_OUTLINE,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[144] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextOutlineItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextOutline" ),
     // Slot Nr. 144 : 10263
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_SHADOW,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[145] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadow" ),
     // Slot Nr. 145 : 10264
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_SHDWCOLOR,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[146] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowColorItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowColor" ),
     // Slot Nr. 146 : 10265
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_SHDWXVAL,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[147] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowXValItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowXVal" ),
     // Slot Nr. 147 : 10266
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_SHDWYVAL,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[148] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowYValItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowYVal" ),
     // Slot Nr. 148 : 10268
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FORMTEXT_HIDEFORM,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[138] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecFormText),SFX_STUB_PTR(GraphicViewShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextHideFormItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"FormatFontWorkClose" ),
     // Slot Nr. 149 : 10280
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GALLERY_FORMATS,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[171] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecGallery),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertGalleryPic" ),
     // Slot Nr. 150 : 10281
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SEARCH_OPTIONS,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[155] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,Execute),SFX_STUB_PTR(GraphicViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       102/*Offset*/, 0, SfxSlotMode::NONE,"SearchOptions" ),
     // Slot Nr. 151 : 10284
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LONG_ULSPACE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[152] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLongULSpaceItem,
                       102/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetLongTopBottomMargin" ),
     // Slot Nr. 152 : 10285
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LONG_LRSPACE,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[202] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLongLRSpaceItem,
                       103/*Offset*/, 0, SfxSlotMode::NONE,"SetLongLeftRightMargin" ),
     // Slot Nr. 153 : 10286
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FRAME_TO_TOP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[154] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       103/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BringToFront" ),
     // Slot Nr. 154 : 10287
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FRAME_TO_BOTTOM,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[158] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       103/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SendToBack" ),
     // Slot Nr. 155 : 10291
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SEARCH_ITEM,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[150] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,Execute),SFX_STUB_PTR(GraphicViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSearchItem,
                       103/*Offset*/, 0, SfxSlotMode::NONE,"SearchProperties" ),
     // Slot Nr. 156 : 10294
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SET_SUPER_SCRIPT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[157] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       103/*Offset*/, 0, SfxSlotMode::NONE,"SuperScript" ),
     // Slot Nr. 157 : 10295
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SET_SUB_SCRIPT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[160] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       103/*Offset*/, 0, SfxSlotMode::NONE,"SubScript" ),
     // Slot Nr. 158 : 10296
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHAR_DLG,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[159] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       103/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontDialog" ),
     // Slot Nr. 159 : 10297
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PARA_DLG,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[164] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       104/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ParagraphDialog" ),
     // Slot Nr. 160 : 10299
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_SHADOW,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[161] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrOnOffItem,
                       104/*Offset*/, 0, SfxSlotMode::NONE,"FillShadow" ),
     // Slot Nr. 161 : 10301
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINEEND_STYLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[173] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       104/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineEndStyle" ),
     // Slot Nr. 162 : 10310
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_AUTOWIDTH,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[163] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       108/*Offset*/, 0, SfxSlotMode::NONE,"AutomaticWidth" ),
     // Slot Nr. 163 : 10311
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TRANSFORM_AUTOHEIGHT,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       108/*Offset*/, 0, SfxSlotMode::NONE,"AutomaticHeight" ),
     // Slot Nr. 164 : 10330
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SCAN,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[165] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       108/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Scan" ),
     // Slot Nr. 165 : 10331
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TWAIN_SELECT,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[166] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       108/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TwainSelect" ),
     // Slot Nr. 166 : 10332
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TWAIN_TRANSFER,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[167] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       108/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TwainTransfer" ),
     // Slot Nr. 167 : 10350
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BMPMASK,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[172] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       108/*Offset*/, 0, SfxSlotMode::NONE,"BmpMask" ),
     // Slot Nr. 168 : 10351
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BMPMASK_PIPETTE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[169] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecBmpMask),SFX_STUB_PTR(GraphicViewShell,GetBmpMaskState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       108/*Offset*/, 0, SfxSlotMode::NONE,"BmpMaskPipette" ),
     // Slot Nr. 169 : 10353
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BMPMASK_EXEC,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[168] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecBmpMask),SFX_STUB_PTR(GraphicViewShell,GetBmpMaskState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       108/*Offset*/, 0, SfxSlotMode::NONE,"BmpMaskExec" ),
     // Slot Nr. 170 : 10361
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HYPERLINK_GETLINK,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[223] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHyperlinkItem,
                       108/*Offset*/, 0, SfxSlotMode::NONE,"Hyperlink" ),
     // Slot Nr. 171 : 10362
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HYPERLINK_SETLINK,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[213] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       108/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetHyperlink" ),
     // Slot Nr. 172 : 10366
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAVIGATOR,SfxGroupId::Navigator,
                       &aGraphicViewShellSlots_Impl[174] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       109/*Offset*/, 0, SfxSlotMode::NONE,"Navigator" ),
     // Slot Nr. 173 : 10367
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_TEXT_FITTOSIZE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[207] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrTextFitToSizeTypeItem,
                       109/*Offset*/, 0, SfxSlotMode::NONE,"TextFitToSize" ),
     // Slot Nr. 174 : 10371
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_IMAP,SfxGroupId::Graphic,
                       &aGraphicViewShellSlots_Impl[176] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       109/*Offset*/, 0, SfxSlotMode::NONE,"ImageMapDialog" ),
     // Slot Nr. 175 : 10374
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_IMAP_EXEC,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[175] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecIMap),SFX_STUB_PTR(GraphicViewShell,GetIMapState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       109/*Offset*/, 0, SfxSlotMode::NONE,"ImageMapExecute" ),
     // Slot Nr. 176 : 10378
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_RECT_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[177] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       109/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Rect_Unfilled" ),
     // Slot Nr. 177 : 10379
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_RECT_ROUND_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[178] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       113/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Rect_Rounded_Unfilled" ),
     // Slot Nr. 178 : 10380
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_SQUARE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[179] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       117/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Square" ),
     // Slot Nr. 179 : 10381
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_SQUARE_ROUND,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[180] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       121/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Square_Rounded" ),
     // Slot Nr. 180 : 10382
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_SQUARE_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[181] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       125/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Square_Unfilled" ),
     // Slot Nr. 181 : 10383
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_SQUARE_ROUND_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[182] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       129/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Square_Rounded_Unfilled" ),
     // Slot Nr. 182 : 10384
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_ELLIPSE_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[183] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       133/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Ellipse_Unfilled" ),
     // Slot Nr. 183 : 10385
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[184] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       137/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Circle" ),
     // Slot Nr. 184 : 10386
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLE_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[185] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       141/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Circle_Unfilled" ),
     // Slot Nr. 185 : 10387
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_PIE_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[186] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       145/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Pie_Unfilled" ),
     // Slot Nr. 186 : 10388
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLEPIE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[187] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       151/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CirclePie" ),
     // Slot Nr. 187 : 10389
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLEPIE_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[188] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       157/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CirclePie_Unfilled" ),
     // Slot Nr. 188 : 10390
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLEARC,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[189] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       163/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CircleArc" ),
     // Slot Nr. 189 : 10391
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CIRCLECUT_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[190] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       169/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CircleCut_Unfilled" ),
     // Slot Nr. 190 : 10392
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_ELLIPSECUT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[191] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       175/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EllipseCut" ),
     // Slot Nr. 191 : 10393
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_ELLIPSECUT_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[192] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       181/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EllipseCut_Unfilled" ),
     // Slot Nr. 192 : 10394
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_XPOLYGON,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[193] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"Polygon_Diagonal" ),
     // Slot Nr. 193 : 10395
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_POLYGON_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[194] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"Polygon_Unfilled" ),
     // Slot Nr. 194 : 10396
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_XPOLYGON_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[195] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"Polygon_Diagonal_Unfilled" ),
     // Slot Nr. 195 : 10397
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_BEZIER_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[196] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"Bezier_Unfilled" ),
     // Slot Nr. 196 : 10398
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_TEXT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[197] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"TextToolbox" ),
     // Slot Nr. 197 : 10399
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_RECTANGLES,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[198] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"RectangleToolbox" ),
     // Slot Nr. 198 : 10400
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_ELLIPSES,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[199] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"EllipseToolbox" ),
     // Slot Nr. 199 : 10401
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_LINES,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[200] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"LineToolbox" ),
     // Slot Nr. 200 : 10402
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_NEXT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[201] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomNext" ),
     // Slot Nr. 201 : 10403
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_PREV,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[203] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPrevious" ),
     // Slot Nr. 202 : 10415
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER_PROTECT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[232] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxProtectItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"RulerProtect" ),
     // Slot Nr. 203 : 10417
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COLOR_CONTROL,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[205] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"ColorControl" ),
     // Slot Nr. 204 : 10429
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TABLEDESIGN,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[106] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTable),SFX_STUB_PTR(GraphicViewShell,GetTableMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"TableDesign" ),
     // Slot Nr. 205 : 10454
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GROUP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[206] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatGroup" ),
     // Slot Nr. 206 : 10455
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_UNGROUP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[209] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatUngroup" ),
     // Slot Nr. 207 : 10456
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SET_DEFAULT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[208] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetDefault" ),
     // Slot Nr. 208 : 10459
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_REMOVE_HYPERLINK,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[266] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       187/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RemoveHyperlink" ),
     // Slot Nr. 209 : 10463
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_FREELINE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[210] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 0, SfxSlotMode::NONE,"Freeline" ),
     // Slot Nr. 210 : 10464
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_FREELINE_NOFILL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[211] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       187/*Offset*/, 5/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Freeline_Unfilled" ),
     // Slot Nr. 211 : 10503
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHARMAP,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[212] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       192/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSymbol" ),
     // Slot Nr. 212 : 10606
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_REDACTED_EXPORT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[214] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"RedactedExportToolbox" ),
     // Slot Nr. 213 : 10628
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FM_CREATE_CONTROL,SfxGroupId::Controls,
                       &aGraphicViewShellSlots_Impl[220] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateControl" ),
     // Slot Nr. 214 : 10644
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_WIN,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[215] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"Window3D" ),
     // Slot Nr. 215 : 10645
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_STATE,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[216] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecEffectWin),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"State3D" ),
     // Slot Nr. 216 : 10646
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_INIT,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[217] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecEffectWin),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"Init3D" ),
     // Slot Nr. 217 : 10647
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_ASSIGN,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[218] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecEffectWin),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"Assign3D" ),
     // Slot Nr. 218 : 10648
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONVERT_TO_3D,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[219] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConvertInto3D" ),
     // Slot Nr. 219 : 10649
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONVERT_TO_3D_LATHE_FAST,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[221] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConvertInto3DLatheFast" ),
     // Slot Nr. 220 : 10765
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FM_CREATE_FIELDCONTROL,SfxGroupId::Controls,
                       &aGraphicViewShellSlots_Impl[311] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateFieldControl" ),
     // Slot Nr. 221 : 10905
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_TEXT_VERTICAL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[222] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"VerticalText" ),
     // Slot Nr. 222 : 10906
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CAPTION_VERTICAL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[229] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"VerticalCaption" ),
     // Slot Nr. 223 : 10912
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_UPPER,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[224] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToUpper" ),
     // Slot Nr. 224 : 10913
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_LOWER,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[225] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToLower" ),
     // Slot Nr. 225 : 10914
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_HALFWIDTH,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[226] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHalfWidth" ),
     // Slot Nr. 226 : 10915
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_FULLWIDTH,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[227] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToFullWidth" ),
     // Slot Nr. 227 : 10916
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_HIRAGANA,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[228] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHiragana" ),
     // Slot Nr. 228 : 10917
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_KATAKANA,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[231] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToKatakana" ),
     // Slot Nr. 229 : 10923
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETUNDOSTRINGS,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[230] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"GetUndoStrings" ),
     // Slot Nr. 230 : 10924
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETREDOSTRINGS,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[233] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"GetRedoStrings" ),
     // Slot Nr. 231 : 10930
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_READONLY_MODE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[293] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ReadOnlyMode" ),
     // Slot Nr. 232 : 10952
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER_TEXT_RIGHT_TO_LEFT,SfxGroupId::Controls,
                       &aGraphicViewShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecRuler),SFX_STUB_PTR(GraphicViewShell,GetRulerState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"TextRTL" ),
     // Slot Nr. 233 : 10955
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OPEN_HYPERLINK,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[234] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenHyperlinkOnCursor" ),
     // Slot Nr. 234 : 10956
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CTLFONT_STATE,SfxGroupId::Application,
                       &aGraphicViewShellSlots_Impl[235] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"CTLFontState" ),
     // Slot Nr. 235 : 10957
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_VERTICALTEXT_STATE,SfxGroupId::Application,
                       &aGraphicViewShellSlots_Impl[236] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"VerticalTextState" ),
     // Slot Nr. 236 : 10958
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OPEN_XML_FILTERSETTINGS,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[237] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenXMLFilterSettings" ),
     // Slot Nr. 237 : 10960
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_TOGGLE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[238] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionToggle" ),
     // Slot Nr. 238 : 10961
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_TILT_DOWN,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[239] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltDown" ),
     // Slot Nr. 239 : 10962
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_TILT_UP,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[240] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltUp" ),
     // Slot Nr. 240 : 10963
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_TILT_LEFT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[241] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltLeft" ),
     // Slot Nr. 241 : 10964
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_TILT_RIGHT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[242] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltRight" ),
     // Slot Nr. 242 : 10965
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_DEPTH_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[243] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionDepthFloater" ),
     // Slot Nr. 243 : 10966
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_DIRECTION_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[244] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionDirectionFloater" ),
     // Slot Nr. 244 : 10967
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_LIGHTING_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[245] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionLightingFloater" ),
     // Slot Nr. 245 : 10968
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_SURFACE_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[246] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       194/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionSurfaceFloater" ),
     // Slot Nr. 246 : 10969
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_3D_COLOR,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[247] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"Extrusion3DColor" ),
     // Slot Nr. 247 : 10970
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_DEPTH,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[248] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxDoubleItem,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionDepth" ),
     // Slot Nr. 248 : 10971
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_DIRECTION,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[249] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionDirection" ),
     // Slot Nr. 249 : 10972
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_PROJECTION,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[250] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionProjection" ),
     // Slot Nr. 250 : 10973
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_LIGHTING_DIRECTION,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[251] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionLightingDirection" ),
     // Slot Nr. 251 : 10974
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_LIGHTING_INTENSITY,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[252] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionLightingIntensity" ),
     // Slot Nr. 252 : 10975
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_SURFACE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[253] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       194/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionSurface" ),
     // Slot Nr. 253 : 10976
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTRUSION_DEPTH_DIALOG,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[254] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxDoubleItem,
                       194/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionDepthDialog" ),
     // Slot Nr. 254 : 10977
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_GALLERY_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[255] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       196/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkGalleryFloater" ),
     // Slot Nr. 255 : 10978
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EDIT_HYPERLINK,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[256] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       196/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditHyperlink" ),
     // Slot Nr. 256 : 10979
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_SHAPE_TYPE,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[257] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       196/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkShapeType" ),
     // Slot Nr. 257 : 10980
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_SAME_LETTER_HEIGHTS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[258] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"FontworkSameLetterHeights" ),
     // Slot Nr. 258 : 10981
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_ALIGNMENT_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[259] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       197/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkAlignmentFloater" ),
     // Slot Nr. 259 : 10982
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_CHARACTER_SPACING_FLOATER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[260] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       197/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkCharacterSpacingFloater" ),
     // Slot Nr. 260 : 10983
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_SHAPE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[261] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"FontworkShape" ),
     // Slot Nr. 261 : 10984
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_ALIGNMENT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[262] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"FontworkAlignment" ),
     // Slot Nr. 262 : 10985
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_CHARACTER_SPACING,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[263] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"FontworkCharacterSpacing" ),
     // Slot Nr. 263 : 10986
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_KERN_CHARACTER_PAIRS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[264] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"FontworkKernCharacterPairs" ),
     // Slot Nr. 264 : 10987
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_FONTWORK,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[265] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"DrawFontwork" ),
     // Slot Nr. 265 : 10988
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_FONTWORK_VERTICAL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[268] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       197/*Offset*/, 0, SfxSlotMode::NONE,"DrawFontworkVertical" ),
     // Slot Nr. 266 : 11042
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GROW_FONT_SIZE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[267] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       197/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Grow" ),
     // Slot Nr. 267 : 11043
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SHRINK_FONT_SIZE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[303] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       197/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Shrink" ),
     // Slot Nr. 268 : 11047
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CS_BASIC,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[269] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       197/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicShapes" ),
     // Slot Nr. 269 : 11048
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CS_SYMBOL,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[270] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       198/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SymbolShapes" ),
     // Slot Nr. 270 : 11049
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CS_ARROW,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[271] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       199/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ArrowShapes" ),
     // Slot Nr. 271 : 11050
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CS_FLOWCHART,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[272] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       200/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FlowChartShapes" ),
     // Slot Nr. 272 : 11051
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CS_CALLOUT,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[273] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       201/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CalloutShapes" ),
     // Slot Nr. 273 : 11052
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CS_STAR,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[274] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       202/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StarShapes" ),
     // Slot Nr. 274 : 11053
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_CS_ID,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[275] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       203/*Offset*/, 0, SfxSlotMode::NONE,"CustomShape" ),
     // Slot Nr. 275 : 11054
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FONTWORK_CHARACTER_SPACING_DIALOG,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[276] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       203/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkCharacterSpacingDialog" ),
     // Slot Nr. 276 : 11056
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_RLM,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[277] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       204/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRLM" ),
     // Slot Nr. 277 : 11057
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_LRM,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[278] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       204/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertLRM" ),
     // Slot Nr. 278 : 11058
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_ZWSP,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[279] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       204/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertZWSP" ),
     // Slot Nr. 279 : 11059
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_ZWNBSP,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[284] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       204/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertZWNBSP" ),
     // Slot Nr. 280 : 11065
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_ZOOMSLIDER,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[396] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomSliderItem,
                       204/*Offset*/, 0, SfxSlotMode::NONE,"ZoomSlider" ),
     // Slot Nr. 281 : 11073
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TABLE_VERT_BOTTOM,SfxGroupId::Table,
                       &aGraphicViewShellSlots_Impl[282] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecutePropPanelAttr),SFX_STUB_PTR(GraphicViewShell,GetStatePropPanelAttr),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       204/*Offset*/, 0, SfxSlotMode::NONE,"CellVertBottom" ),
     // Slot Nr. 282 : 11074
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TABLE_VERT_CENTER,SfxGroupId::Table,
                       &aGraphicViewShellSlots_Impl[283] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecutePropPanelAttr),SFX_STUB_PTR(GraphicViewShell,GetStatePropPanelAttr),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       204/*Offset*/, 0, SfxSlotMode::NONE,"CellVertCenter" ),
     // Slot Nr. 283 : 11075
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TABLE_VERT_NONE,SfxGroupId::Table,
                       &aGraphicViewShellSlots_Impl[281] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecutePropPanelAttr),SFX_STUB_PTR(GraphicViewShell,GetStatePropPanelAttr),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       204/*Offset*/, 0, SfxSlotMode::NONE,"CellVertTop" ),
     // Slot Nr. 284 : 11093
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_MIRROR,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[297] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       204/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Mirror" ),
     // Slot Nr. 285 : 11094
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETE_POSTIT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[286] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       205/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteAnnotation" ),
     // Slot Nr. 286 : 11095
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NEXT_POSTIT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[287] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       206/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NextAnnotation" ),
     // Slot Nr. 287 : 11096
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PREVIOUS_POSTIT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[288] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       206/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PreviousAnnotation" ),
     // Slot Nr. 288 : 11097
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETEALL_POSTIT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[289] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       206/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteAllAnnotation" ),
     // Slot Nr. 289 : 11098
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TOGGLE_NOTES,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[290] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       206/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowAnnotations" ),
     // Slot Nr. 290 : 11099
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_REPLYTO_POSTIT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[291] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       206/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ReplyToAnnotation" ),
     // Slot Nr. 291 : 11100
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETEALLBYAUTHOR_POSTIT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[292] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       208/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteAllAnnotationByAuthor" ),
     // Slot Nr. 292 : 11101
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_POSTIT,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[285] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecuteAnnotation),SFX_STUB_PTR(GraphicViewShell,GetAnnotationState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       208/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertAnnotation" ),
     // Slot Nr. 293 : 11102
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_SENTENCE_CASE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[294] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToSentenceCase" ),
     // Slot Nr. 294 : 11103
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_TITLE_CASE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[295] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToTitleCase" ),
     // Slot Nr. 295 : 11104
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_TOGGLE_CASE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[296] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToToggleCase" ),
     // Slot Nr. 296 : 11105
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TRANSLITERATE_ROTATE_CASE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[344] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupportRotate),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseRotateCase" ),
     // Slot Nr. 297 : 11113
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EXTERNAL_EDIT,SfxGroupId::Graphic,
                       &aGraphicViewShellSlots_Impl[298] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"ExternalEdit" ),
     // Slot Nr. 298 : 11115
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHANGE_PICTURE,SfxGroupId::Graphic,
                       &aGraphicViewShellSlots_Impl[299] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangePicture" ),
     // Slot Nr. 299 : 11116
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SAVE_GRAPHIC,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[300] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveGraphic" ),
     // Slot Nr. 300 : 11117
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COMPRESS_GRAPHIC,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[301] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CompressGraphic" ),
     // Slot Nr. 301 : 11118
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FRAME_UP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[302] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectForwardOne" ),
     // Slot Nr. 302 : 11119
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FRAME_DOWN,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[306] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectBackOne" ),
     // Slot Nr. 303 : 11124
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_TRANSPARENCE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[304] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillTransparence" ),
     // Slot Nr. 304 : 11125
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_FILL_FLOATTRANSPARENCE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[305] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillFloatTransparenceItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillFloatTransparence" ),
     // Slot Nr. 305 : 11126
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_TRANSPARENCE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[308] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"LineTransparence" ),
     // Slot Nr. 306 : 11127
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FLIP_HORIZONTAL,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[307] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FlipHorizontal" ),
     // Slot Nr. 307 : 11128
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_FLIP_VERTICAL,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[310] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FlipVertical" ),
     // Slot Nr. 308 : 11129
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_JOINT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[309] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineJointItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"LineJoint" ),
     // Slot Nr. 309 : 11130
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_LINE_CAP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[314] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineCapItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"LineCap" ),
     // Slot Nr. 310 : 11133
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHAR_DLG_EFFECT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[313] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontEffectsDialog" ),
     // Slot Nr. 311 : 11136
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_SVX_SET_NUMBER,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[312] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"SetNumber" ),
     // Slot Nr. 312 : 11137
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_SVX_SET_BULLET,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[333] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"SetBullet" ),
     // Slot Nr. 313 : 11147
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_CROP,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[324] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       211/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Crop" ),
     // Slot Nr. 314 : 11148
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_SHADOW_TRANSPARENCE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[315] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrPercentItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowTransparency" ),
     // Slot Nr. 315 : 11149
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_SHADOW_COLOR,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[316] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XColorItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowColor" ),
     // Slot Nr. 316 : 11150
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_SHADOW_XDISTANCE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[317] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrMetricItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowXDistance" ),
     // Slot Nr. 317 : 11151
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_SHADOW_YDISTANCE,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[319] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrMetricItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowYDistance" ),
     // Slot Nr. 318 : 11152
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_COLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[320] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillColorItem,
                       211/*Offset*/, 0, SfxSlotMode::NONE,"FillPageColor" ),
     // Slot Nr. 319 : 11153
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_BACK_COLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[338] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBackgroundColorItem,
                       211/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CharBackColor" ),
     // Slot Nr. 320 : 11154
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_GRADIENT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[321] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillGradientItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"FillPageGradient" ),
     // Slot Nr. 321 : 11155
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_HATCH,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[322] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillHatchItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"FillPageHatch" ),
     // Slot Nr. 322 : 11156
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_BITMAP,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[323] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillBitmapItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"FillPageBitmap" ),
     // Slot Nr. 323 : 11157
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_PAGE_FILLSTYLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,SetPageProperties),SFX_STUB_PTR(GraphicViewShell,GetPageProperties),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillStyleItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"FillPageStyle" ),
     // Slot Nr. 324 : 11159
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_ARROW_CIRCLE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[325] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowCircle" ),
     // Slot Nr. 325 : 11160
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_ARROW_SQUARE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[326] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowSquare" ),
     // Slot Nr. 326 : 11161
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_CIRCLE_ARROW,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[327] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineCircleArrow" ),
     // Slot Nr. 327 : 11162
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_SQUARE_ARROW,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[328] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineSquareArrow" ),
     // Slot Nr. 328 : 11163
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_ARROWS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[329] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineArrows" ),
     // Slot Nr. 329 : 11164
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_ARROWS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[330] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"ArrowsToolbox" ),
     // Slot Nr. 330 : 11165
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_ARROW_START,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[331] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowStart" ),
     // Slot Nr. 331 : 11166
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINE_ARROW_END,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[332] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowEnd" ),
     // Slot Nr. 332 : 11167
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAW_MEASURELINE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[334] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"MeasureLine" ),
     // Slot Nr. 333 : 11171
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CLASSIFICATION_DIALOG,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[374] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClassificationDialog" ),
     // Slot Nr. 334 : 11176
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MEASURE_DLG,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[335] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MeasureAttributes" ),
     // Slot Nr. 335 : 11191
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_QRCODE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[336] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertQrCode" ),
     // Slot Nr. 336 : 11192
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EDIT_QRCODE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[337] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditQrCode" ),
     // Slot Nr. 337 : 11193
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COPY_HYPERLINK_LOCATION,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[343] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"CopyHyperlinkLocation" ),
     // Slot Nr. 338 : 11195
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ULINE_VAL_NONE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[339] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineNone" ),
     // Slot Nr. 339 : 11196
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ULINE_VAL_SINGLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[340] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineSingle" ),
     // Slot Nr. 340 : 11197
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ULINE_VAL_DOUBLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[341] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDouble" ),
     // Slot Nr. 341 : 11198
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ULINE_VAL_DOTTED,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[342] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDotted" ),
     // Slot Nr. 342 : 11568
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_CHAR_OVERLINE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[346] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecChar),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxOverlineItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"Overline" ),
     // Slot Nr. 343 : 12021
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_AUTOSPELL_CHECK,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[350] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"SpellOnline" ),
     // Slot Nr. 344 : 12087
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ATTR_YEAR2000,SfxGroupId::Intern,
                       &aGraphicViewShellSlots_Impl[345] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       213/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Year2000" ),
     // Slot Nr. 345 : 12094
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OPT_LOCALE_CHANGED,SfxGroupId::Special,
                       &aGraphicViewShellSlots_Impl[377] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 0, SfxSlotMode::NONE,"OptionsLocaleChanged" ),
     // Slot Nr. 346 : 20138
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_NUM_BULLET_ON,SfxGroupId::Enumeration,
                       &aGraphicViewShellSlots_Impl[347] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       213/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefaultBullet" ),
     // Slot Nr. 347 : 20144
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_NUM_NUMBERING_ON,SfxGroupId::Enumeration,
                       &aGraphicViewShellSlots_Impl[348] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       214/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefaultNumbering" ),
     // Slot Nr. 348 : 20220
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_BUL_NUM_RULE_INDEX,SfxGroupId::Enumeration,
                       &aGraphicViewShellSlots_Impl[349] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       215/*Offset*/, 0, SfxSlotMode::NONE,"CurrentBulletListType" ),
     // Slot Nr. 349 : 20221
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_NUM_NUM_RULE_INDEX,SfxGroupId::Enumeration,
                       &aGraphicViewShellSlots_Impl[522] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       215/*Offset*/, 0, SfxSlotMode::NONE,"CurrentNumListType" ),
     // Slot Nr. 350 : 20343
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_INSERT_SOFT_HYPHEN,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[351] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSoftHyphen" ),
     // Slot Nr. 351 : 20344
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_INSERT_HARD_SPACE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[352] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertNonBreakingSpace" ),
     // Slot Nr. 352 : 20345
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_INSERT_NNBSP,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[353] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertNarrowNobreakSpace" ),
     // Slot Nr. 353 : 20385
     SFX_NEW_SLOT_ARG( GraphicViewShell,FN_INSERT_HARDHYPHEN,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[354] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertHardHyphen" ),
     // Slot Nr. 354 : 27002
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PAGESETUP,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[355] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PageSetup" ),
     // Slot Nr. 355 : 27004
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COPYOBJECTS,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[356] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CopyObjects" ),
     // Slot Nr. 356 : 27005
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MANAGE_LINKS,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[357] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ManageLinks" ),
     // Slot Nr. 357 : 27008
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONVERT_TO_3D_LATHE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[358] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConvertInto3DLathe" ),
     // Slot Nr. 358 : 27009
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWINGMODE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[359] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       215/*Offset*/, 0, SfxSlotMode::NONE,"DrawingMode" ),
     // Slot Nr. 359 : 27014
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERTPAGE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[360] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       215/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPage" ),
     // Slot Nr. 360 : 27015
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERTFILE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[361] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       219/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ImportFromFile" ),
     // Slot Nr. 361 : 27017
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_PANNING,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[362] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPanning" ),
     // Slot Nr. 362 : 27022
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_POSITION,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[363] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0, SfxSlotMode::NONE,"ObjectPosition" ),
     // Slot Nr. 363 : 27025
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TEXT_COMBINE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[364] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TextCombine" ),
     // Slot Nr. 364 : 27026
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_COMBINE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[365] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Combine" ),
     // Slot Nr. 365 : 27027
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAME_GROUP,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[366] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NameGroup" ),
     // Slot Nr. 366 : 27028
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_CONNECTORS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[367] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorToolbox" ),
     // Slot Nr. 367 : 27031
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MOREFRONT,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[368] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Forward" ),
     // Slot Nr. 368 : 27032
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MOREBACK,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[369] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Backward" ),
     // Slot Nr. 369 : 27033
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_TITLE_DESCRIPTION,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[370] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectTitleDescription" ),
     // Slot Nr. 370 : 27034
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_VERTICAL,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[371] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MirrorVert" ),
     // Slot Nr. 371 : 27035
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HORIZONTAL,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[372] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MirrorHorz" ),
     // Slot Nr. 372 : 27036
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHANGEBEZIER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[373] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeBezier" ),
     // Slot Nr. 373 : 27037
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CHANGEPOLYGON,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[375] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangePolygon" ),
     // Slot Nr. 374 : 27038
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CAPTUREPOINT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CapturePoint" ),
     // Slot Nr. 375 : 27039
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERTLAYER,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[376] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       221/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertLayer" ),
     // Slot Nr. 376 : 27040
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RULER,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[379] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       225/*Offset*/, 0, SfxSlotMode::NONE,"ShowRuler" ),
     // Slot Nr. 377 : 27042
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SWITCHPAGE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[378] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       225/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SwitchPage" ),
     // Slot Nr. 378 : 27044
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SWITCHLAYER,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[536] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       227/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SwitchLayer" ),
     // Slot Nr. 379 : 27045
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MODIFYLAYER,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[380] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       228/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ModifyLayer" ),
     // Slot Nr. 380 : 27046
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PAGEMODE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[381] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       232/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PageMode" ),
     // Slot Nr. 381 : 27047
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LAYERMODE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[382] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       234/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LayerMode" ),
     // Slot Nr. 382 : 27050
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MASTERPAGE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[387] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       236/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MasterPage" ),
     // Slot Nr. 383 : 27051
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GO_TO_NEXT_PAGE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[383] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecGoToNextPage),SFX_STUB_PTR(GraphicViewShell,GetStateGoToNextPage),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NextPage" ),
     // Slot Nr. 384 : 27052
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GO_TO_PREVIOUS_PAGE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[384] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecGoToPreviousPage),SFX_STUB_PTR(GraphicViewShell,GetStateGoToPreviousPage),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PreviousPage" ),
     // Slot Nr. 385 : 27053
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GO_TO_FIRST_PAGE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[385] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecGoToFirstPage),SFX_STUB_PTR(GraphicViewShell,GetStateGoToFirstPage),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FirstPage" ),
     // Slot Nr. 386 : 27054
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GO_TO_LAST_PAGE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[386] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecGoToLastPage),SFX_STUB_PTR(GraphicViewShell,GetStateGoToLastPage),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LastPage" ),
     // Slot Nr. 387 : 27061
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TOOL_CONNECTOR,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[388] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"Connector" ),
     // Slot Nr. 388 : 27064
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PRESENTATION_LAYOUT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[389] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PresentationLayout" ),
     // Slot Nr. 389 : 27071
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINEEND_POLYGON,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[390] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineEndPolygon" ),
     // Slot Nr. 390 : 27074
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SWITCH_POINTEDIT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[391] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SwitchPointEdit" ),
     // Slot Nr. 391 : 27076
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TEXTEDIT,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[393] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ToolEdit" ),
     // Slot Nr. 392 : 27077
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SET_SNAPITEM,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[412] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetSnapItemState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"SetSnapItem" ),
     // Slot Nr. 393 : 27080
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETE_PAGE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[394] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeletePage" ),
     // Slot Nr. 394 : 27081
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETE_LAYER,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[395] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteLayer" ),
     // Slot Nr. 395 : 27082
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DISMANTLE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[398] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Dismantle" ),
     // Slot Nr. 396 : 27086
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STATUS_PAGE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[397] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecStatusBar),SFX_STUB_PTR(GraphicViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"PageStatus" ),
     // Slot Nr. 397 : 27087
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_STATUS_LAYOUT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecStatusBar),SFX_STUB_PTR(GraphicViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"LayoutStatus" ),
     // Slot Nr. 398 : 27090
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_CROOK_ROTATE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[399] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"CrookRotate" ),
     // Slot Nr. 399 : 27091
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_CROOK_SLANT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[400] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"CrookSlant" ),
     // Slot Nr. 400 : 27092
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_CROOK_STRETCH,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[401] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"CrookStretch" ),
     // Slot Nr. 401 : 27093
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECT,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[402] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Connect" ),
     // Slot Nr. 402 : 27094
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BREAK,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[403] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Break" ),
     // Slot Nr. 403 : 27095
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_CHOOSE_MODE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[404] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"AdvancedMode" ),
     // Slot Nr. 404 : 27096
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ENTER_GROUP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[405] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EnterGroup" ),
     // Slot Nr. 405 : 27097
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LEAVE_GROUP,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[406] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LeaveGroup" ),
     // Slot Nr. 406 : 27098
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SIZE_PAGE_WIDTH,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[407] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPageWidth" ),
     // Slot Nr. 407 : 27099
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SIZE_OPTIMAL,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[408] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomObjects" ),
     // Slot Nr. 408 : 27100
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_TRANSPARENCE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[409] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InteractiveTransparence" ),
     // Slot Nr. 409 : 27101
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_GRADIENT,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[410] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InteractiveGradient" ),
     // Slot Nr. 410 : 27106
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_MATH,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[411] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertMath" ),
     // Slot Nr. 411 : 27107
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_SHEAR,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[413] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"Shear" ),
     // Slot Nr. 412 : 27108
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETE_SNAPITEM,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[392] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetSnapItemState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"DeleteSnapItem" ),
     // Slot Nr. 413 : 27109
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OBJECT_CLOSE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[414] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CloseObject" ),
     // Slot Nr. 414 : 27114
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SIZE_VISAREA,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[415] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomVisArea" ),
     // Slot Nr. 415 : 27117
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_REVERSE_ORDER,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[416] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ReverseOrder" ),
     // Slot Nr. 416 : 27119
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_ARROW_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[417] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorArrowStart" ),
     // Slot Nr. 417 : 27120
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_ARROW_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[418] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorArrowEnd" ),
     // Slot Nr. 418 : 27121
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_ARROWS,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[419] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorArrows" ),
     // Slot Nr. 419 : 27122
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CIRCLE_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[420] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCircleStart" ),
     // Slot Nr. 420 : 27123
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CIRCLE_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[421] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCircleEnd" ),
     // Slot Nr. 421 : 27124
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CIRCLES,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[422] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCircles" ),
     // Slot Nr. 422 : 27125
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[423] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLine" ),
     // Slot Nr. 423 : 27126
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE_ARROW_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[424] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLineArrowStart" ),
     // Slot Nr. 424 : 27127
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE_ARROW_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[425] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLineArrowEnd" ),
     // Slot Nr. 425 : 27128
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE_ARROWS,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[426] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLineArrows" ),
     // Slot Nr. 426 : 27129
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE_CIRCLE_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[427] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLineCircleStart" ),
     // Slot Nr. 427 : 27130
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE_CIRCLE_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[428] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLineCircleEnd" ),
     // Slot Nr. 428 : 27131
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINE_CIRCLES,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[429] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLineCircles" ),
     // Slot Nr. 429 : 27132
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[430] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurve" ),
     // Slot Nr. 430 : 27133
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE_ARROW_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[431] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurveArrowStart" ),
     // Slot Nr. 431 : 27134
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE_ARROW_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[432] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurveArrowEnd" ),
     // Slot Nr. 432 : 27135
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE_ARROWS,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[433] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurveArrows" ),
     // Slot Nr. 433 : 27136
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE_CIRCLE_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[434] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurveCircleStart" ),
     // Slot Nr. 434 : 27137
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE_CIRCLE_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[435] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurveCircleEnd" ),
     // Slot Nr. 435 : 27138
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_CURVE_CIRCLES,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[436] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorCurveCircles" ),
     // Slot Nr. 436 : 27139
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[437] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLines" ),
     // Slot Nr. 437 : 27140
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES_ARROW_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[438] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLinesArrowStart" ),
     // Slot Nr. 438 : 27141
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES_ARROW_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[439] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLinesArrowEnd" ),
     // Slot Nr. 439 : 27142
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES_ARROWS,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[440] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLinesArrows" ),
     // Slot Nr. 440 : 27143
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES_CIRCLE_START,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[441] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLinesCircleStart" ),
     // Slot Nr. 441 : 27144
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES_CIRCLE_END,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[442] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLinesCircleEnd" ),
     // Slot Nr. 442 : 27145
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTOR_LINES_CIRCLES,SfxGroupId::Connector,
                       &aGraphicViewShellSlots_Impl[443] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ConnectorLinesCircles" ),
     // Slot Nr. 443 : 27146
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BEHIND_OBJ,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[456] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BehindObject" ),
     // Slot Nr. 444 : 27151
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SOLID_CREATE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[445] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"SolidCreate" ),
     // Slot Nr. 445 : 27152
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HELPLINES_USE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[446] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"HelplinesUse" ),
     // Slot Nr. 446 : 27153
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HELPLINES_MOVE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[447] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"HelplinesMove" ),
     // Slot Nr. 447 : 27154
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GRID_USE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[448] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"GridUse" ),
     // Slot Nr. 448 : 27155
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SNAP_BORDER,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[449] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"SnapBorder" ),
     // Slot Nr. 449 : 27156
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SNAP_FRAME,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[450] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"SnapFrame" ),
     // Slot Nr. 450 : 27157
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SNAP_POINTS,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[451] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"SnapPoints" ),
     // Slot Nr. 451 : 27158
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_QUICKEDIT,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[452] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"QuickEdit" ),
     // Slot Nr. 452 : 27159
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PICK_THROUGH,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[453] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"PickThrough" ),
     // Slot Nr. 453 : 27169
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DOUBLECLICK_TEXTEDIT,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[454] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"DoubleClickTextEdit" ),
     // Slot Nr. 454 : 27170
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CLICK_CHANGE_ROTATION,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[455] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"ClickChangeRotation" ),
     // Slot Nr. 455 : 27179
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HELPLINES_VISIBLE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[520] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"HelplinesVisible" ),
     // Slot Nr. 456 : 27180
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HELPLINES_FRONT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[457] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"HelplinesFront" ),
     // Slot Nr. 457 : 27181
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BEFORE_OBJ,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[458] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BeforeObject" ),
     // Slot Nr. 458 : 27248
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GRID_FRONT,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[459] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       237/*Offset*/, 0, SfxSlotMode::NONE,"GridFront" ),
     // Slot Nr. 459 : 27250
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MOVETO,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[460] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       237/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MoveTo" ),
     // Slot Nr. 460 : 27251
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LINETO,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[461] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       239/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineTo" ),
     // Slot Nr. 461 : 27252
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_BEZIERTO,SfxGroupId::NONE,
                       &aGraphicViewShellSlots_Impl[462] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       241/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BezierTo" ),
     // Slot Nr. 462 : 27253
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PAGESIZE,SfxGroupId::Application,
                       &aGraphicViewShellSlots_Impl[463] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       243/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PageSize" ),
     // Slot Nr. 463 : 27254
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PAGEMARGIN,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[473] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       246/*Offset*/, 5/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PageMargin" ),
     // Slot Nr. 464 : 27259
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETRED,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[465] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       251/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GetRed" ),
     // Slot Nr. 465 : 27260
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETBLUE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[466] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       252/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GetBlue" ),
     // Slot Nr. 466 : 27261
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETGREEN,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[467] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       253/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GetGreen" ),
     // Slot Nr. 467 : 27262
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETFILLSTYLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[468] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       254/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetFillStyle" ),
     // Slot Nr. 468 : 27263
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETFILLCOLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[469] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       255/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetFillColor" ),
     // Slot Nr. 469 : 27264
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GRADIENT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[470] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       258/*Offset*/, 8/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Gradient" ),
     // Slot Nr. 470 : 27265
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETLINESTYLE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[471] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       266/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetLineStyle" ),
     // Slot Nr. 471 : 27266
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETLINEWIDTH,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[472] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       267/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetLineWidth" ),
     // Slot Nr. 472 : 27267
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DASH,SfxGroupId::Document,
                       &aGraphicViewShellSlots_Impl[475] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       268/*Offset*/, 7/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Dash" ),
     // Slot Nr. 473 : 27268
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RENAMEPAGE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[474] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       275/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenamePage" ),
     // Slot Nr. 474 : 27269
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RENAMELAYER,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[486] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       276/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenameLayer" ),
     // Slot Nr. 475 : 27270
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HATCH,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[476] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       276/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Hatch" ),
     // Slot Nr. 476 : 27271
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETFILLSTYLE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[477] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       280/*Offset*/, 0, SfxSlotMode::NONE,"GetFillStyle" ),
     // Slot Nr. 477 : 27272
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETLINESTYLE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[478] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       280/*Offset*/, 0, SfxSlotMode::NONE,"GetLineStyle" ),
     // Slot Nr. 478 : 27273
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GETLINEWIDTH,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[479] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       280/*Offset*/, 0, SfxSlotMode::NONE,"GetLineWidth" ),
     // Slot Nr. 479 : 27274
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETLINECOLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[480] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       280/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineColor" ),
     // Slot Nr. 480 : 27275
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETHATCHCOLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[481] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       283/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HatchColor" ),
     // Slot Nr. 481 : 27276
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETGRADSTARTCOLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[482] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       287/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GradientStartColor" ),
     // Slot Nr. 482 : 27277
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SETGRADENDCOLOR,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[483] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       291/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GradientEndColor" ),
     // Slot Nr. 483 : 27278
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SELECTGRADIENT,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[484] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       295/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectGradient" ),
     // Slot Nr. 484 : 27279
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SELECTHATCH,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[485] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       296/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectHatch" ),
     // Slot Nr. 485 : 27280
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_UNSELECT,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[464] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,AttrExec),SFX_STUB_PTR(GraphicViewShell,AttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       297/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeSelect" ),
     // Slot Nr. 486 : 27281
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TEXTATTR_DLG,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[487] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       297/*Offset*/, 7/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TextAttributes" ),
     // Slot Nr. 487 : 27282
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ORIGINAL_SIZE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[488] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       304/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OriginalSize" ),
     // Slot Nr. 488 : 27285
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TEXT_FITTOSIZE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[489] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"TextFitToSizeTool" ),
     // Slot Nr. 489 : 27286
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_TEXT_FITTOSIZE_VERTICAL,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[495] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"VerticalTextFitToSizeTool" ),
     // Slot Nr. 490 : 27287
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAVIGATOR_PAGENAME,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[491] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecNavigatorWin),SFX_STUB_PTR(GraphicViewShell,GetNavigatorWinState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"NavigatorPageName" ),
     // Slot Nr. 491 : 27288
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAVIGATOR_STATE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[492] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecNavigatorWin),SFX_STUB_PTR(GraphicViewShell,GetNavigatorWinState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"NavigatorState" ),
     // Slot Nr. 492 : 27289
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAVIGATOR_INIT,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[493] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecNavigatorWin),SFX_STUB_PTR(GraphicViewShell,GetNavigatorWinState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"NavigatorInit" ),
     // Slot Nr. 493 : 27292
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAVIGATOR_PAGE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[494] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecNavigatorWin),SFX_STUB_PTR(GraphicViewShell,GetNavigatorWinState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"NavigatorPage" ),
     // Slot Nr. 494 : 27293
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_NAVIGATOR_OBJECT,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[490] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecNavigatorWin),SFX_STUB_PTR(GraphicViewShell,GetNavigatorWinState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"NavigatorObject" ),
     // Slot Nr. 495 : 27295
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_3D_OBJECTS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[496] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Objects3DToolbox" ),
     // Slot Nr. 496 : 27296
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_CUBE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[497] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Cube" ),
     // Slot Nr. 497 : 27297
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_SPHERE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[498] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Sphere" ),
     // Slot Nr. 498 : 27298
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_CYLINDER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[499] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Cylinder" ),
     // Slot Nr. 499 : 27299
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_CONE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[500] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Cone" ),
     // Slot Nr. 500 : 27300
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_PYRAMID,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[501] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Cyramid" ),
     // Slot Nr. 501 : 27301
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_EDITMODE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[502] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueEditMode" ),
     // Slot Nr. 502 : 27302
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_INSERT_POINT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[503] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueInsertPoint" ),
     // Slot Nr. 503 : 27303
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_PERCENT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[504] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GluePercent" ),
     // Slot Nr. 504 : 27304
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_ESCDIR,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[505] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueEscapeDirection" ),
     // Slot Nr. 505 : 27305
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_HORZALIGN_CENTER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[506] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueHorzAlignCenter" ),
     // Slot Nr. 506 : 27306
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_HORZALIGN_LEFT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[507] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueHorzAlignLeft" ),
     // Slot Nr. 507 : 27307
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_HORZALIGN_RIGHT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[508] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueHorzAlignRight" ),
     // Slot Nr. 508 : 27308
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_VERTALIGN_CENTER,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[509] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueVertAlignCenter" ),
     // Slot Nr. 509 : 27309
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_VERTALIGN_TOP,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[510] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueVertAlignTop" ),
     // Slot Nr. 510 : 27310
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_VERTALIGN_BOTTOM,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[511] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueVertAlignBottom" ),
     // Slot Nr. 511 : 27311
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_SHELL,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[512] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Shell3D" ),
     // Slot Nr. 512 : 27312
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_TORUS,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[513] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Torus" ),
     // Slot Nr. 513 : 27313
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_3D_HALF_SPHERE,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[514] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"HalfSphere" ),
     // Slot Nr. 514 : 27314
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_ESCDIR_LEFT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[515] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueEscapeDirectionLeft" ),
     // Slot Nr. 515 : 27315
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_ESCDIR_RIGHT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[516] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueEscapeDirectionRight" ),
     // Slot Nr. 516 : 27316
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_ESCDIR_TOP,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[517] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueEscapeDirectionTop" ),
     // Slot Nr. 517 : 27317
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GLUE_ESCDIR_BOTTOM,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[518] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GlueEscapeDirectionBottom" ),
     // Slot Nr. 518 : 27318
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DRAWTBX_INSERT,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[519] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"InsertToolbox" ),
     // Slot Nr. 519 : 27319
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_POLYGON_MORPHING,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[521] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       304/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Morphing" ),
     // Slot Nr. 520 : 27322
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_GRID_VISIBLE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[444] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecOptionsBar),SFX_STUB_PTR(GraphicViewShell,GetOptionsBarState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"GridVisible" ),
     // Slot Nr. 521 : 27338
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTION_DLG,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[523] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       304/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConnectorAttributes" ),
     // Slot Nr. 522 : 27340
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HYPHENATION,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"Hyphenation" ),
     // Slot Nr. 523 : 27341
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONNECTION_NEW_ROUTING,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[524] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       304/*Offset*/, 0, SfxSlotMode::NONE,"NewRouting" ),
     // Slot Nr. 524 : 27342
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DUPLICATE_PAGE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[525] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       304/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DuplicatePage" ),
     // Slot Nr. 525 : 27345
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_LEAVE_ALL_GROUPS,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[526] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       308/*Offset*/, 0, SfxSlotMode::NONE,"LeaveAllGroups" ),
     // Slot Nr. 526 : 27352
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERTPAGE_QUICK,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[527] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPageQuick" ),
     // Slot Nr. 527 : 27356
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_PAGE_TITLE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[528] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPageTitleField" ),
     // Slot Nr. 528 : 27357
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_DATE_VAR,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[529] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertDateFieldVar" ),
     // Slot Nr. 529 : 27358
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_DATE_FIX,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[530] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertDateFieldFix" ),
     // Slot Nr. 530 : 27359
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_TIME_VAR,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[531] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertTimeFieldVar" ),
     // Slot Nr. 531 : 27360
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_TIME_FIX,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[532] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertTimeFieldFix" ),
     // Slot Nr. 532 : 27361
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_PAGE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[533] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPageField" ),
     // Slot Nr. 533 : 27362
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MODIFY_FIELD,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[534] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ModifyField" ),
     // Slot Nr. 534 : 27363
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_FILE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[535] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFileField" ),
     // Slot Nr. 535 : 27364
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_AUTHOR,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[539] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertAuthorField" ),
     // Slot Nr. 536 : 27366
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OUTPUT_QUALITY_COLOR,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[537] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       308/*Offset*/, 0, SfxSlotMode::NONE,"OutputQualityColor" ),
     // Slot Nr. 537 : 27367
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OUTPUT_QUALITY_GRAYSCALE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[538] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       308/*Offset*/, 0, SfxSlotMode::NONE,"OutputQualityGrayscale" ),
     // Slot Nr. 538 : 27368
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OUTPUT_QUALITY_BLACKWHITE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[543] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       308/*Offset*/, 0, SfxSlotMode::NONE,"OutputQualityBlackWhite" ),
     // Slot Nr. 539 : 27373
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_FLD_PAGES,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[540] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPagesField" ),
     // Slot Nr. 540 : 27378
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONVERT_TO_BITMAP,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[541] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConvertIntoBitmap" ),
     // Slot Nr. 541 : 27379
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONVERT_TO_METAFILE,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[542] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConvertIntoMetaFile" ),
     // Slot Nr. 542 : 27381
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CONVERT_TO_CONTOUR,SfxGroupId::Drawing,
                       &aGraphicViewShellSlots_Impl[544] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"convert_to_contour" ),
     // Slot Nr. 543 : 27400
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OUTPUT_QUALITY_CONTRAST,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetCtrlState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       308/*Offset*/, 0, SfxSlotMode::NONE,"OutputQualityContrast" ),
     // Slot Nr. 544 : 27405
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RENAMEPAGE_QUICK,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[545] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenamePageQuick" ),
     // Slot Nr. 545 : 27407
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HEADER_AND_FOOTER,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[546] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HeaderAndFooter" ),
     // Slot Nr. 546 : 27408
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_MASTER_LAYOUTS,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[547] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MasterLayouts" ),
     // Slot Nr. 547 : 27411
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_PAGE_NUMBER,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[548] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPageNumber" ),
     // Slot Nr. 548 : 27412
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_DATE_TIME,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[549] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertDateAndTime" ),
     // Slot Nr. 549 : 27422
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SELECT_BACKGROUND,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[550] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       308/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectBackground" ),
     // Slot Nr. 550 : 27423
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SAVE_BACKGROUND,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[551] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       312/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveBackground" ),
     // Slot Nr. 551 : 27431
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_INSERT_MASTER_PAGE,SfxGroupId::Insert,
                       &aGraphicViewShellSlots_Impl[552] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       312/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertMasterPage" ),
     // Slot Nr. 552 : 27432
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DELETE_MASTER_PAGE,SfxGroupId::Edit,
                       &aGraphicViewShellSlots_Impl[553] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteMasterPage" ),
     // Slot Nr. 553 : 27433
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_RENAME_MASTER_PAGE,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[554] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenameMasterPage" ),
     // Slot Nr. 554 : 27434
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_CLOSE_MASTER_VIEW,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[555] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuSupport),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CloseMasterView" ),
     // Slot Nr. 555 : 27436
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DISPLAY_MASTER_BACKGROUND,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[556] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       316/*Offset*/, 0, SfxSlotMode::NONE,"DisplayMasterBackground" ),
     // Slot Nr. 556 : 27437
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_DISPLAY_MASTER_OBJECTS,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[557] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       316/*Offset*/, 0, SfxSlotMode::NONE,"DisplayMasterObjects" ),
     // Slot Nr. 557 : 27440
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_OUTLINE_TEXT_AUTOFIT,SfxGroupId::Format,
                       &aGraphicViewShellSlots_Impl[558] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       316/*Offset*/, 0, SfxSlotMode::NONE,"TextAutoFitToSize" ),
     // Slot Nr. 558 : 27447
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_ZOOM_MODE,SfxGroupId::View,
                       &aGraphicViewShellSlots_Impl[559] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuPermanent),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomMode" ),
     // Slot Nr. 559 : 27448
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_HIDE_LAST_LEVEL,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[560] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideLastLevel" ),
     // Slot Nr. 560 : 27449
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_SHOW_NEXT_LEVEL,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[561] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowNextLevel" ),
     // Slot Nr. 561 : 27450
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_PRESENTATION_MINIMIZER,SfxGroupId::Options,
                       &aGraphicViewShellSlots_Impl[562] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,FuTemporary),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PresentationMinimizer" ),
     // Slot Nr. 562 : 27451
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_REGENERATE_DIAGRAM,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[563] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RegenerateDiagram" ),
     // Slot Nr. 563 : 27452
     SFX_NEW_SLOT_ARG( GraphicViewShell,SID_EDIT_DIAGRAM,SfxGroupId::Modify,
                       &aGraphicViewShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicViewShell,ExecCtrl),SFX_STUB_PTR(GraphicViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       316/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditDiagram" )
    };
#endif

/************************************************************/
#ifdef ShellClass_OutlineView
#undef ShellClass
#undef ShellClass_OutlineView
#define ShellClass OutlineView
#endif

/************************************************************/
#ifdef ShellClass_OutlineViewShell
#undef ShellClass
#undef ShellClass_OutlineViewShell
#define ShellClass OutlineViewShell
static SfxFormalArgument aOutlineViewShellArgs_Impl[] =
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
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_NEW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_EDIT },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_DELETE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_NEW_BY_EXAMPLE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_UPDATE_BY_EXAMPLE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_HIDE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_SHOW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Page", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Page", FN_PARAM_1 },
     { (const SfxType*) &aSvxHyperlinkItem_Impl, "Hyperlink", SID_HYPERLINK_SETLINK },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbols", SID_CHARMAP },
     { (const SfxType*) &aSfxStringItem_Impl, "FontName", SID_ATTR_SPECIALCHAR },
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", ID_VAL_DUMMY1 },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", ID_VAL_DUMMY2 }
    };

SFX_EXEC_STUB(OutlineViewShell,FuSupport)
SFX_STATE_STUB(OutlineViewShell,GetMenuState)
SFX_EXEC_STUB(OutlineViewShell,FuTemporaryModify)
SFX_EXEC_STUB(OutlineViewShell,Execute)
SFX_EXEC_STUB(OutlineViewShell,FuTemporary)
SFX_STATE_STUB(OutlineViewShell,GetAttrState)
SFX_EXEC_STUB(OutlineViewShell,ExecCtrl)
SFX_STATE_STUB(OutlineViewShell,GetCtrlState)
SFX_STATE_STUB(OutlineViewShell,GetStatusBarState)
SFX_STATE_STUB(OutlineViewShell,GetState)
SFX_EXEC_STUB(OutlineViewShell,FuPermanent)
SFX_EXEC_STUB(OutlineViewShell,ExecStatusBar)

static SfxSlot aOutlineViewShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5314
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PASTE_UNFORMATTED,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteUnformatted" ),
     // Slot Nr. 1 : 5405
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_EMOJI_CONTROL,SfxGroupId::Special,
                       &aOutlineViewShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"EmojiControl" ),
     // Slot Nr. 2 : 5406
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_CHARMAP_CONTROL,SfxGroupId::Special,
                       &aOutlineViewShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"CharmapControl" ),
     // Slot Nr. 3 : 5502
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SAVEASDOC,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 12/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveAs" ),
     // Slot Nr. 4 : 5505
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SAVEDOC,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,Execute),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       12/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Save" ),
     // Slot Nr. 5 : 5542
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_FAMILY2,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"ParaStyle" ),
     // Slot Nr. 6 : 5543
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_FAMILY3,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"FrameStyle" ),
     // Slot Nr. 7 : 5545
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_FAMILY5,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"ListStyle" ),
     // Slot Nr. 8 : 5549
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_NEW,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewStyle" ),
     // Slot Nr. 9 : 5550
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_EDIT,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       18/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditStyle" ),
     // Slot Nr. 10 : 5551
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_DELETE,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       20/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteStyle" ),
     // Slot Nr. 11 : 5553
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_FAMILY,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       22/*Offset*/, 0, SfxSlotMode::NONE,"ActualStyleFamily" ),
     // Slot Nr. 12 : 5554
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_WATERCAN,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       22/*Offset*/, 0, SfxSlotMode::NONE,"StyleWatercanMode" ),
     // Slot Nr. 13 : 5555
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_NEW_BY_EXAMPLE,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleNewByExample" ),
     // Slot Nr. 14 : 5556
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_UPDATE_BY_EXAMPLE,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleUpdateByExample" ),
     // Slot Nr. 15 : 5700
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_REDO,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       26/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 16 : 5701
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_UNDO,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       28/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 17 : 5710
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_CUT,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 18 : 5711
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_COPY,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 19 : 5712
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PASTE,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 20 : 5713
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_DELETE,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 21 : 5723
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SELECTALL,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 22 : 6588
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_MAIL_SCROLLBODY_PAGEDOWN,SfxGroupId::Explorer,
                       &aOutlineViewShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,ExecCtrl),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ScrollBodyPageDown" ),
     // Slot Nr. 23 : 6603
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_HIDE,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideStyle" ),
     // Slot Nr. 24 : 6604
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STYLE_SHOW,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       34/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowStyle" ),
     // Slot Nr. 25 : 10000
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ATTR_ZOOM,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetStatusBarState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"Zoom" ),
     // Slot Nr. 26 : 10096
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ZOOM_TOOLBOX,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"ZoomToolBox" ),
     // Slot Nr. 27 : 10097
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ZOOM_OUT,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPlus" ),
     // Slot Nr. 28 : 10098
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ZOOM_IN,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomMinus" ),
     // Slot Nr. 29 : 10099
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SIZE_REAL,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Zoom100Percent" ),
     // Slot Nr. 30 : 10154
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OUTLINE_FORMAT,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"OutlineFormat" ),
     // Slot Nr. 31 : 10155
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OUTLINE_COLLAPSE_ALL,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineCollapseAll" ),
     // Slot Nr. 32 : 10156
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OUTLINE_BULLET,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineBullet" ),
     // Slot Nr. 33 : 10157
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PRESENTATION,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Presentation" ),
     // Slot Nr. 34 : 10158
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PRESENTATION_CURRENT_SLIDE,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PresentationCurrentSlide" ),
     // Slot Nr. 35 : 10231
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OUTLINE_COLLAPSE,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineCollapse" ),
     // Slot Nr. 36 : 10232
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OUTLINE_EXPAND_ALL,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineExpandAll" ),
     // Slot Nr. 37 : 10233
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OUTLINE_EXPAND,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineExpand" ),
     // Slot Nr. 38 : 10243
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SPELL_DIALOG,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,Execute),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"SpellDialog" ),
     // Slot Nr. 39 : 10245
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_THESAURUS,SfxGroupId::Options,
                       &aOutlineViewShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusDialog" ),
     // Slot Nr. 40 : 10281
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SEARCH_OPTIONS,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,Execute),SFX_STUB_PTR(OutlineViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"SearchOptions" ),
     // Slot Nr. 41 : 10291
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SEARCH_ITEM,SfxGroupId::NONE,
                       &aOutlineViewShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,Execute),SFX_STUB_PTR(OutlineViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSearchItem,
                       37/*Offset*/, 0, SfxSlotMode::NONE,"SearchProperties" ),
     // Slot Nr. 42 : 10296
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_CHAR_DLG,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       37/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontDialog" ),
     // Slot Nr. 43 : 10361
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_HYPERLINK_GETLINK,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHyperlinkItem,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"Hyperlink" ),
     // Slot Nr. 44 : 10362
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_HYPERLINK_SETLINK,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetHyperlink" ),
     // Slot Nr. 45 : 10402
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ZOOM_NEXT,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomNext" ),
     // Slot Nr. 46 : 10403
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ZOOM_PREV,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomPrevious" ),
     // Slot Nr. 47 : 10456
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SET_DEFAULT,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetDefault" ),
     // Slot Nr. 48 : 10503
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_CHARMAP,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       39/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSymbol" ),
     // Slot Nr. 49 : 10912
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_UPPER,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToUpper" ),
     // Slot Nr. 50 : 10913
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_LOWER,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToLower" ),
     // Slot Nr. 51 : 10914
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_HALFWIDTH,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHalfWidth" ),
     // Slot Nr. 52 : 10915
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_FULLWIDTH,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToFullWidth" ),
     // Slot Nr. 53 : 10916
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_HIRAGANA,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHiragana" ),
     // Slot Nr. 54 : 10917
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_KATAKANA,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToKatakana" ),
     // Slot Nr. 55 : 10923
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_GETUNDOSTRINGS,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"GetUndoStrings" ),
     // Slot Nr. 56 : 10924
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_GETREDOSTRINGS,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"GetRedoStrings" ),
     // Slot Nr. 57 : 10930
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_READONLY_MODE,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ReadOnlyMode" ),
     // Slot Nr. 58 : 11056
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_RLM,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRLM" ),
     // Slot Nr. 59 : 11057
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_LRM,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertLRM" ),
     // Slot Nr. 60 : 11058
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_ZWSP,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertZWSP" ),
     // Slot Nr. 61 : 11059
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_ZWNBSP,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertZWNBSP" ),
     // Slot Nr. 62 : 11065
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_ATTR_ZOOMSLIDER,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomSliderItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ZoomSlider" ),
     // Slot Nr. 63 : 11102
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_SENTENCE_CASE,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToSentenceCase" ),
     // Slot Nr. 64 : 11103
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_TITLE_CASE,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToTitleCase" ),
     // Slot Nr. 65 : 11104
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_TRANSLITERATE_TOGGLE_CASE,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToToggleCase" ),
     // Slot Nr. 66 : 11133
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_CHAR_DLG_EFFECT,SfxGroupId::Format,
                       &aOutlineViewShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontEffectsDialog" ),
     // Slot Nr. 67 : 11136
     SFX_NEW_SLOT_ARG( OutlineViewShell,FN_SVX_SET_NUMBER,SfxGroupId::Special,
                       &aOutlineViewShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"SetNumber" ),
     // Slot Nr. 68 : 11137
     SFX_NEW_SLOT_ARG( OutlineViewShell,FN_SVX_SET_BULLET,SfxGroupId::Special,
                       &aOutlineViewShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"SetBullet" ),
     // Slot Nr. 69 : 12021
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_AUTOSPELL_CHECK,SfxGroupId::Options,
                       &aOutlineViewShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"SpellOnline" ),
     // Slot Nr. 70 : 12094
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_OPT_LOCALE_CHANGED,SfxGroupId::Special,
                       &aOutlineViewShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,ExecCtrl),SFX_STUB_PTR(OutlineViewShell,GetCtrlState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"OptionsLocaleChanged" ),
     // Slot Nr. 71 : 20343
     SFX_NEW_SLOT_ARG( OutlineViewShell,FN_INSERT_SOFT_HYPHEN,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSoftHyphen" ),
     // Slot Nr. 72 : 20344
     SFX_NEW_SLOT_ARG( OutlineViewShell,FN_INSERT_HARD_SPACE,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertNonBreakingSpace" ),
     // Slot Nr. 73 : 20345
     SFX_NEW_SLOT_ARG( OutlineViewShell,FN_INSERT_NNBSP,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertNarrowNobreakSpace" ),
     // Slot Nr. 74 : 20385
     SFX_NEW_SLOT_ARG( OutlineViewShell,FN_INSERT_HARDHYPHEN,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertHardHyphen" ),
     // Slot Nr. 75 : 27009
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_DRAWINGMODE,SfxGroupId::Drawing,
                       &aOutlineViewShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"DrawingMode" ),
     // Slot Nr. 76 : 27015
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERTFILE,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ImportFromFile" ),
     // Slot Nr. 77 : 27040
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_RULER,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuSupport),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       43/*Offset*/, 0, SfxSlotMode::NONE,"ShowRuler" ),
     // Slot Nr. 78 : 27041
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_EDIT_OUTLINER,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuPermanent),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditOutline" ),
     // Slot Nr. 79 : 27086
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STATUS_PAGE,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,ExecStatusBar),SFX_STUB_PTR(OutlineViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       43/*Offset*/, 0, SfxSlotMode::NONE,"PageStatus" ),
     // Slot Nr. 80 : 27087
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_STATUS_LAYOUT,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,ExecStatusBar),SFX_STUB_PTR(OutlineViewShell,GetStatusBarState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       43/*Offset*/, 0, SfxSlotMode::NONE,"LayoutStatus" ),
     // Slot Nr. 81 : 27113
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PRESENTATIONOBJECT,SfxGroupId::Modify,
                       &aOutlineViewShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ModifyPresentationObject" ),
     // Slot Nr. 82 : 27114
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SIZE_VISAREA,SfxGroupId::Document,
                       &aOutlineViewShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomVisArea" ),
     // Slot Nr. 83 : 27257
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_COLORVIEW,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       43/*Offset*/, 0, SfxSlotMode::NONE,"ColorView" ),
     // Slot Nr. 84 : 27329
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PREVIEW_STATE,SfxGroupId::View,
                       &aOutlineViewShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       43/*Offset*/, 0, SfxSlotMode::NONE,"PreviewState" ),
     // Slot Nr. 85 : 27336
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PHOTOALBUM,SfxGroupId::Options,
                       &aOutlineViewShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PhotoAlbumDialog" ),
     // Slot Nr. 86 : 27337
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_REMOTE_DLG,SfxGroupId::Options,
                       &aOutlineViewShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Remote" ),
     // Slot Nr. 87 : 27339
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_PRESENTATION_DLG,SfxGroupId::Options,
                       &aOutlineViewShellSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PresentationDialog" ),
     // Slot Nr. 88 : 27343
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_EXPAND_PAGE,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExpandPage" ),
     // Slot Nr. 89 : 27344
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_SUMMARY_PAGE,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SummaryPage" ),
     // Slot Nr. 90 : 27356
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_PAGE_TITLE,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPageTitleField" ),
     // Slot Nr. 91 : 27357
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_DATE_VAR,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertDateFieldVar" ),
     // Slot Nr. 92 : 27358
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_DATE_FIX,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertDateFieldFix" ),
     // Slot Nr. 93 : 27359
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_TIME_VAR,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertTimeFieldVar" ),
     // Slot Nr. 94 : 27360
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_TIME_FIX,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertTimeFieldFix" ),
     // Slot Nr. 95 : 27361
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_PAGE,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPageField" ),
     // Slot Nr. 96 : 27362
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_MODIFY_FIELD,SfxGroupId::Edit,
                       &aOutlineViewShellSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ModifyField" ),
     // Slot Nr. 97 : 27363
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_FILE,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFileField" ),
     // Slot Nr. 98 : 27364
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_AUTHOR,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertAuthorField" ),
     // Slot Nr. 99 : 27365
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_CUSTOMSHOW_DLG,SfxGroupId::Options,
                       &aOutlineViewShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporary),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CustomShowDialog" ),
     // Slot Nr. 100 : 27373
     SFX_NEW_SLOT_ARG( OutlineViewShell,SID_INSERT_FLD_PAGES,SfxGroupId::Insert,
                       &aOutlineViewShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(OutlineViewShell,FuTemporaryModify),SFX_STUB_PTR(OutlineViewShell,GetMenuState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertPagesField" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TextObjectBar
#undef ShellClass
#undef ShellClass_TextObjectBar
#define ShellClass TextObjectBar
static SfxFormalArgument aTextObjectBarArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "Template", SID_STYLE_APPLY },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "FamilyName", SID_STYLE_FAMILYNAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Style", SID_APPLY_STYLE },
     { (const SfxType*) &aSfxStringItem_Impl, "WordReplace", SID_THES },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxColorItem_Impl, "Color", SID_ATTR_CHAR_COLOR },
     { (const SfxType*) &aSvxLRSpaceItem_Impl, "LRSpace", SID_ATTR_PARA_LRSPACE },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxBackgroundColorItem_Impl, "CharBackColor", SID_ATTR_CHAR_BACK_COLOR },
     { (const SfxType*) &aSfxBoolItem_Impl, "On", FN_PARAM_1 }
    };

SFX_EXEC_STUB(TextObjectBar,Execute)
SFX_STATE_STUB(TextObjectBar,GetAttrState)
SFX_STATE_STUB(TextObjectBar,GetCharState)

static SfxSlot aTextObjectBarSlots_Impl[] =
    {
     // Slot Nr. 0 : 5552
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_STYLE_APPLY,SfxGroupId::Document,
                       &aTextObjectBarSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       0/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleApply" ),
     // Slot Nr. 1 : 5698
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_THES,SfxGroupId::Text,
                       &aTextObjectBarSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusFromContext" ),
     // Slot Nr. 2 : 10007
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_FONT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"CharFontName" ),
     // Slot Nr. 3 : 10008
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_POSTURE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPostureItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"Italic" ),
     // Slot Nr. 4 : 10009
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_WEIGHT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxWeightItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"Bold" ),
     // Slot Nr. 5 : 10010
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_SHADOWED,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxShadowedItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"Shadowed" ),
     // Slot Nr. 6 : 10012
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_CONTOUR,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxContourItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"OutlineFont" ),
     // Slot Nr. 7 : 10013
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_STRIKEOUT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCrossedOutItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"Strikeout" ),
     // Slot Nr. 8 : 10014
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_UNDERLINE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxUnderlineItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"Underline" ),
     // Slot Nr. 9 : 10015
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_FONTHEIGHT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontHeightItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"FontHeight" ),
     // Slot Nr. 10 : 10017
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_COLOR,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       5/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Color" ),
     // Slot Nr. 11 : 10018
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_KERNING,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetCharState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxKerningItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"Spacing" ),
     // Slot Nr. 12 : 10019
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_CASEMAP,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCaseMapItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"CaseMap" ),
     // Slot Nr. 13 : 10028
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_ADJUST_LEFT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"LeftPara" ),
     // Slot Nr. 14 : 10029
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_ADJUST_RIGHT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"RightPara" ),
     // Slot Nr. 15 : 10030
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_ADJUST_CENTER,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"CenterPara" ),
     // Slot Nr. 16 : 10031
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_ADJUST_BLOCK,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"JustifyPara" ),
     // Slot Nr. 17 : 10034
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_LINESPACE_10,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"SpacePara1" ),
     // Slot Nr. 18 : 10035
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_LINESPACE_15,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"SpacePara15" ),
     // Slot Nr. 19 : 10036
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_LINESPACE_20,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       7/*Offset*/, 0, SfxSlotMode::NONE,"SpacePara2" ),
     // Slot Nr. 20 : 10043
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_LRSPACE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLRSpaceItem,
                       7/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LeftRightParaMargin" ),
     // Slot Nr. 21 : 10068
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_HANGING_INDENT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"HangingIndent" ),
     // Slot Nr. 22 : 10150
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_OUTLINE_UP,SfxGroupId::View,
                       &aTextObjectBarSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineUp" ),
     // Slot Nr. 23 : 10151
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_OUTLINE_DOWN,SfxGroupId::View,
                       &aTextObjectBarSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineDown" ),
     // Slot Nr. 24 : 10152
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_OUTLINE_LEFT,SfxGroupId::View,
                       &aTextObjectBarSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineLeft" ),
     // Slot Nr. 25 : 10153
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_OUTLINE_RIGHT,SfxGroupId::View,
                       &aTextObjectBarSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OutlineRight" ),
     // Slot Nr. 26 : 10294
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_SET_SUPER_SCRIPT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"SuperScript" ),
     // Slot Nr. 27 : 10295
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_SET_SUB_SCRIPT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"SubScript" ),
     // Slot Nr. 28 : 10461
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_DEC_INDENT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DecrementIndent" ),
     // Slot Nr. 29 : 10462
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_INC_INDENT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"IncrementIndent" ),
     // Slot Nr. 30 : 10907
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_TEXTDIRECTION_LEFT_TO_RIGHT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"TextdirectionLeftToRight" ),
     // Slot Nr. 31 : 10908
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_TEXTDIRECTION_TOP_TO_BOTTOM,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"TextdirectionTopToBottom" ),
     // Slot Nr. 32 : 10950
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_LEFT_TO_RIGHT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"ParaLeftToRight" ),
     // Slot Nr. 33 : 10951
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_PARA_RIGHT_TO_LEFT,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"ParaRightToLeft" ),
     // Slot Nr. 34 : 11042
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_GROW_FONT_SIZE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Grow" ),
     // Slot Nr. 35 : 11043
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_SHRINK_FONT_SIZE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Shrink" ),
     // Slot Nr. 36 : 11145
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_PARASPACE_INCREASE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ParaspaceIncrease" ),
     // Slot Nr. 37 : 11146
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_PARASPACE_DECREASE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ParaspaceDecrease" ),
     // Slot Nr. 38 : 11153
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_BACK_COLOR,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBackgroundColorItem,
                       8/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CharBackColor" ),
     // Slot Nr. 39 : 11195
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ULINE_VAL_NONE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineNone" ),
     // Slot Nr. 40 : 11196
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ULINE_VAL_SINGLE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineSingle" ),
     // Slot Nr. 41 : 11197
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ULINE_VAL_DOUBLE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDouble" ),
     // Slot Nr. 42 : 11198
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ULINE_VAL_DOTTED,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDotted" ),
     // Slot Nr. 43 : 11568
     SFX_NEW_SLOT_ARG( TextObjectBar,SID_ATTR_CHAR_OVERLINE,SfxGroupId::Format,
                       &aTextObjectBarSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxOverlineItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"Overline" ),
     // Slot Nr. 44 : 20138
     SFX_NEW_SLOT_ARG( TextObjectBar,FN_NUM_BULLET_ON,SfxGroupId::Enumeration,
                       &aTextObjectBarSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(TextObjectBar,Execute),SFX_STUB_PTR(TextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefaultBullet" )
    };
#endif

/************************************************************/
#ifdef ShellClass_BezierObjectBar
#undef ShellClass
#undef ShellClass_BezierObjectBar
#define ShellClass BezierObjectBar
static SfxFormalArgument aBezierObjectBarArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };

SFX_EXEC_STUB(BezierObjectBar,Execute)
SFX_STATE_STUB(BezierObjectBar,GetAttrState)

static SfxSlot aBezierObjectBarSlots_Impl[] =
    {
     // Slot Nr. 0 : 10119
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_INSERT,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierInsert" ),
     // Slot Nr. 1 : 10120
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_DELETE,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierDelete" ),
     // Slot Nr. 2 : 10121
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_MOVE,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierMove" ),
     // Slot Nr. 3 : 10122
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_CLOSE,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierClose" ),
     // Slot Nr. 4 : 10123
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_SMOOTH,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierSmooth" ),
     // Slot Nr. 5 : 10127
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_CUTLINE,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierCutLine" ),
     // Slot Nr. 6 : 27030
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_ELIMINATE_POINTS,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierEliminatePoints" ),
     // Slot Nr. 7 : 27065
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_CONVERT,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierConvert" ),
     // Slot Nr. 8 : 27066
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_EDGE,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierEdge" ),
     // Slot Nr. 9 : 27067
     SFX_NEW_SLOT_ARG( BezierObjectBar,SID_BEZIER_SYMMTR,SfxGroupId::Drawing,
                       &aBezierObjectBarSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(BezierObjectBar,Execute),SFX_STUB_PTR(BezierObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"BezierSymmetric" )
    };
#endif

/************************************************************/
#ifdef ShellClass_DrawDocument
#undef ShellClass
#undef ShellClass_DrawDocument
#define ShellClass DrawDocument
#endif

/************************************************************/
#ifdef ShellClass_GraphicDocument
#undef ShellClass
#undef ShellClass_GraphicDocument
#define ShellClass GraphicDocument
#endif

/************************************************************/
#ifdef ShellClass_GraphicDocShell
#undef ShellClass
#undef ShellClass_GraphicDocShell
#define ShellClass GraphicDocShell
static SfxFormalArgument aGraphicDocShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxBoolItem_Impl, "saveChanges", SID_CLOSEDOC_SAVE },
     { (const SfxType*) &aSfxStringItem_Impl, "fileName", SID_CLOSEDOC_FILENAME },
     { (const SfxType*) &aSfxStringItem_Impl, "File", SID_NOTEBOOKBAR },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSfxBoolItem_Impl, "Quiet", SID_SEARCH_QUIET },
     { (const SfxType*) &aSfxStringItem_Impl, "Language", SID_LANGUAGE_STATUS }
    };

SFX_EXEC_STUB(GraphicDocShell,Execute)
SFX_STATE_STUB(GraphicDocShell,GetState)

static SfxSlot aGraphicDocShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5503
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_CLOSEDOC,SfxGroupId::Document,
                       &aGraphicDocShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CloseDoc" ),
     // Slot Nr. 1 : 6583
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_VERSION,SfxGroupId::Document,
                       &aGraphicDocShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"VersionDialog" ),
     // Slot Nr. 2 : 10022
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_ATTR_CHAR_FONTLIST,SfxGroupId::Document,
                       &aGraphicDocShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontNameList" ),
     // Slot Nr. 3 : 10338
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_NOTEBOOKBAR,SfxGroupId::View,
                       &aGraphicDocShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Notebookbar" ),
     // Slot Nr. 4 : 10441
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_GET_COLORLIST,SfxGroupId::Edit,
                       &aGraphicDocShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       3/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GetColorTable" ),
     // Slot Nr. 5 : 10500
     SFX_NEW_SLOT_ARG( GraphicDocShell,FID_SEARCH_ON,SfxGroupId::Document,
                       &aGraphicDocShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       3/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SearchOn" ),
     // Slot Nr. 6 : 10501
     SFX_NEW_SLOT_ARG( GraphicDocShell,FID_SEARCH_OFF,SfxGroupId::Document,
                       &aGraphicDocShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SearchOff" ),
     // Slot Nr. 7 : 10502
     SFX_NEW_SLOT_ARG( GraphicDocShell,FID_SEARCH_NOW,SfxGroupId::Document,
                       &aGraphicDocShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExecuteSearch" ),
     // Slot Nr. 8 : 10959
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_HANGUL_HANJA_CONVERSION,SfxGroupId::Options,
                       &aGraphicDocShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       7/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HangulHanjaConversion" ),
     // Slot Nr. 9 : 11016
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_CHINESE_CONVERSION,SfxGroupId::Options,
                       &aGraphicDocShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       7/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChineseConversion" ),
     // Slot Nr. 10 : 11067
     SFX_NEW_SLOT_ARG( GraphicDocShell,SID_LANGUAGE_STATUS,SfxGroupId::Format,
                       &aGraphicDocShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicDocShell,Execute),SFX_STUB_PTR(GraphicDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       7/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LanguageStatus" )
    };
#endif

/************************************************************/
#ifdef ShellClass_GraphicObjectBar
#undef ShellClass
#undef ShellClass_GraphicObjectBar
#define ShellClass GraphicObjectBar
static SfxFormalArgument aGraphicObjectBarArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };

SFX_EXEC_STUB(GraphicObjectBar,ExecuteFilter)
SFX_STATE_STUB(GraphicObjectBar,GetFilterState)
SFX_EXEC_STUB(GraphicObjectBar,Execute)
SFX_STATE_STUB(GraphicObjectBar,GetAttrState)

static SfxSlot aGraphicObjectBarSlots_Impl[] =
    {
     // Slot Nr. 0 : 10469
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GraphicFilterToolbox" ),
     // Slot Nr. 1 : 10470
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_INVERT,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterInvert" ),
     // Slot Nr. 2 : 10471
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_SMOOTH,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSmooth" ),
     // Slot Nr. 3 : 10472
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_SHARPEN,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSharpen" ),
     // Slot Nr. 4 : 10473
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_REMOVENOISE,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterRemoveNoise" ),
     // Slot Nr. 5 : 10474
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_SOBEL,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSobel" ),
     // Slot Nr. 6 : 10475
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_MOSAIC,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterMosaic" ),
     // Slot Nr. 7 : 10476
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_EMBOSS,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterRelief" ),
     // Slot Nr. 8 : 10477
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_POSTER,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterPoster" ),
     // Slot Nr. 9 : 10478
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_POPART,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterPopart" ),
     // Slot Nr. 10 : 10479
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_SEPIA,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSepia" ),
     // Slot Nr. 11 : 10480
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_GRFFILTER_SOLARIZE,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,ExecuteFilter),SFX_STUB_PTR(GraphicObjectBar,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSolarize" ),
     // Slot Nr. 12 : 10863
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_LUMINANCE,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafLuminance" ),
     // Slot Nr. 13 : 10864
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_CONTRAST,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafContrast" ),
     // Slot Nr. 14 : 10865
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_RED,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafRed" ),
     // Slot Nr. 15 : 10866
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_GREEN,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafGreen" ),
     // Slot Nr. 16 : 10867
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_BLUE,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafBlue" ),
     // Slot Nr. 17 : 10868
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_GAMMA,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafGamma" ),
     // Slot Nr. 18 : 10869
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_TRANSPARENCE,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafTransparence" ),
     // Slot Nr. 19 : 10870
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_INVERT,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafInvert" ),
     // Slot Nr. 20 : 10871
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_MODE,SfxGroupId::Format,
                       &aGraphicObjectBarSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafMode" ),
     // Slot Nr. 21 : 10883
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_ATTR_GRAF_CROP,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GrafAttrCrop" ),
     // Slot Nr. 22 : 11044
     SFX_NEW_SLOT_ARG( GraphicObjectBar,SID_COLOR_SETTINGS,SfxGroupId::Modify,
                       &aGraphicObjectBarSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(GraphicObjectBar,Execute),SFX_STUB_PTR(GraphicObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"ColorSettings" )
    };
#endif

/************************************************************/
#ifdef ShellClass_MediaObjectBar
#undef ShellClass
#undef ShellClass_MediaObjectBar
#define ShellClass MediaObjectBar
static SfxFormalArgument aMediaObjectBarArgs_Impl[] =
    {
     { (const SfxType*) &aavmedia_MediaItem_Impl, "AVMediaToolBox", SID_AVMEDIA_TOOLBOX }
    };

SFX_EXEC_STUB(MediaObjectBar,Execute)
SFX_STATE_STUB(MediaObjectBar,GetState)

static SfxSlot aMediaObjectBarSlots_Impl[] =
    {
     // Slot Nr. 0 : 6693
     SFX_NEW_SLOT_ARG( MediaObjectBar,SID_AVMEDIA_TOOLBOX,SfxGroupId::Application,
                       &aMediaObjectBarSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(MediaObjectBar,Execute),SFX_STUB_PTR(MediaObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       avmedia_MediaItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AVMediaToolBox" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TableObjectBar
#undef ShellClass
#undef ShellClass_TableObjectBar
#define ShellClass TableObjectBar
static SfxFormalArgument aTableObjectBarArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "BorderType", SID_TABLE_BORDER_TYPE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Index", SID_TABLE_BORDER_INDEX },
     { (const SfxType*) &aSfxInt32Item_Impl, "Offset", SID_TABLE_BORDER_OFFSET },
     { (const SfxType*) &aSvxBoxItem_Impl, "OuterBorder", SID_ATTR_BORDER_OUTER },
     { (const SfxType*) &aSvxBoxInfoItem_Impl, "InnerBorder", SID_ATTR_BORDER_INNER },
     { (const SfxType*) &aSfxInt32Item_Impl, "Amount", SID_TABLE_SPLIT_CELLS },
     { (const SfxType*) &aSfxBoolItem_Impl, "Horizontal", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Proportional", FN_PARAM_2 }
    };

SFX_EXEC_STUB(TableObjectBar,Execute)
SFX_STATE_STUB(TableObjectBar,GetAttrState)
SFX_STATE_STUB(TableObjectBar,GetState)

static SfxSlot aTableObjectBarSlots_Impl[] =
    {
     // Slot Nr. 0 : 10106
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_CHANGE_CURRENT_BORDER_POSITION,SfxGroupId::NONE,
                       &aTableObjectBarSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableChangeCurrentBorderPosition" ),
     // Slot Nr. 1 : 10188
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_ATTR_BORDER,SfxGroupId::Format,
                       &aTableObjectBarSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBoxItem,
                       3/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetBorderStyle" ),
     // Slot Nr. 2 : 10201
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_FRAME_LINESTYLE,SfxGroupId::Frame,
                       &aTableObjectBarSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"LineStyle" ),
     // Slot Nr. 3 : 10202
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_FRAME_LINECOLOR,SfxGroupId::Frame,
                       &aTableObjectBarSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       5/*Offset*/, 0, SfxSlotMode::NONE,"FrameLineColor" ),
     // Slot Nr. 4 : 10418
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_COL_BEFORE,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumnsBefore" ),
     // Slot Nr. 5 : 10419
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_COL_AFTER,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumnsAfter" ),
     // Slot Nr. 6 : 10420
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_ROW_BEFORE,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRowsBefore" ),
     // Slot Nr. 7 : 10421
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_ROW_AFTER,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRowsAfter" ),
     // Slot Nr. 8 : 10426
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_COL_DLG,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumnDialog" ),
     // Slot Nr. 9 : 10427
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_ROW_DLG,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRowDialog" ),
     // Slot Nr. 10 : 11070
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_MERGE_CELLS,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MergeCells" ),
     // Slot Nr. 11 : 11071
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_SPLIT_CELLS,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SplitCell" ),
     // Slot Nr. 12 : 11072
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_OPTIMIZE_TABLE,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"OptimizeTable" ),
     // Slot Nr. 13 : 11073
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_VERT_BOTTOM,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"CellVertBottom" ),
     // Slot Nr. 14 : 11074
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_VERT_CENTER,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"CellVertCenter" ),
     // Slot Nr. 15 : 11075
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_VERT_NONE,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       8/*Offset*/, 0, SfxSlotMode::NONE,"CellVertTop" ),
     // Slot Nr. 16 : 11076
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_ROW,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRows" ),
     // Slot Nr. 17 : 11077
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_INSERT_COL,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumns" ),
     // Slot Nr. 18 : 11078
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_DELETE_ROW,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteRows" ),
     // Slot Nr. 19 : 11079
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_DELETE_COL,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteColumns" ),
     // Slot Nr. 20 : 11080
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_SELECT_ALL,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectTable" ),
     // Slot Nr. 21 : 11081
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_SELECT_COL,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EntireColumn" ),
     // Slot Nr. 22 : 11082
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_SELECT_ROW,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EntireRow" ),
     // Slot Nr. 23 : 11083
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_FORMAT_TABLE_DLG,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableDialog" ),
     // Slot Nr. 24 : 11084
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_SORT_DIALOG,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableSort" ),
     // Slot Nr. 25 : 11085
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_AUTOSUM,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AutoSum" ),
     // Slot Nr. 26 : 11086
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_DISTRIBUTE_COLUMNS,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DistributeColumns" ),
     // Slot Nr. 27 : 11087
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_DISTRIBUTE_ROWS,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DistributeRows" ),
     // Slot Nr. 28 : 11185
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_MINIMAL_COLUMN_WIDTH,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetMinimalColumnWidth" ),
     // Slot Nr. 29 : 11186
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_MINIMAL_ROW_HEIGHT,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetMinimalRowHeight" ),
     // Slot Nr. 30 : 11187
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_OPTIMAL_COLUMN_WIDTH,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetOptimalColumnWidth" ),
     // Slot Nr. 31 : 11188
     SFX_NEW_SLOT_ARG( TableObjectBar,SID_TABLE_OPTIMAL_ROW_HEIGHT,SfxGroupId::Table,
                       &aTableObjectBarSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(TableObjectBar,Execute),SFX_STUB_PTR(TableObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetOptimalRowHeight" )
    };
#endif
