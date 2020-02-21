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
extern SFX2_DLLPUBLIC SfxType0 aSfxBoolItem_Impl;
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
extern SFX2_DLLPUBLIC SfxType0 aSfxVoidItem_Impl;
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
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt16Item_Impl;
/************************************************************/
extern SFX2_DLLPUBLIC SfxType0 aSfxUInt32Item_Impl;
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
extern SfxType0 aSfxIntegerListItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSfxIntegerListItem_Impl =
    {
     createSfxPoolItem<SfxIntegerListItem>, &typeid(SfxIntegerListItem), 0
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
extern SfxType4 aSvxBrushItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSvxBrushItem_Impl =
    {
     createSfxPoolItem<SvxBrushItem>, &typeid(SvxBrushItem), 4, { {MID_GRAPHIC_TRANSPARENT,"Transparent"}, {MID_BACK_COLOR,"BackColor"}, {MID_GRAPHIC_FILTER,"Filtername"}, {MID_GRAPHIC_POSITION,"Position"} }
    };
#endif

/************************************************************/
extern SfxType4 aSvxShadowItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSvxShadowItem_Impl =
    {
     createSfxPoolItem<SvxShadowItem>, &typeid(SvxShadowItem), 4, { {MID_LOCATION,"Location"}, {MID_WIDTH,"Width"}, {MID_TRANSPARENT,"IsTransparent"}, {MID_BG_COLOR,"Color"} }
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
extern SfxType4 aSvxMarginItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aSvxMarginItem_Impl =
    {
     createSfxPoolItem<SvxMarginItem>, &typeid(SvxMarginItem), 4, { {MID_MARGIN_L_MARGIN,"LeftMargin"}, {MID_MARGIN_R_MARGIN,"RightMargin"}, {MID_MARGIN_UP_MARGIN,"UpperMargin"}, {MID_MARGIN_LO_MARGIN,"LowerMargin"} }
    };
#endif

/************************************************************/
extern SfxType0 aSvxRotateModeItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxRotateModeItem_Impl =
    {
     createSfxPoolItem<SvxRotateModeItem>, &typeid(SvxRotateModeItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxCharReliefItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxCharReliefItem_Impl =
    {
     createSfxPoolItem<SvxCharReliefItem>, &typeid(SvxCharReliefItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxHorJustifyItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxHorJustifyItem_Impl =
    {
     createSfxPoolItem<SvxHorJustifyItem>, &typeid(SvxHorJustifyItem), 0
    };
#endif

/************************************************************/
extern SfxType0 aSvxVerJustifyItem_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType0 aSvxVerJustifyItem_Impl =
    {
     createSfxPoolItem<SvxVerJustifyItem>, &typeid(SvxVerJustifyItem), 0
    };
#endif

/************************************************************/
extern SfxType4 aScProtectionAttr_Impl;
#ifdef SFX_TYPEMAP
#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS) || defined(LINUX))) || STATIC_LINKING)
__attribute__((__weak__))
#endif
SfxType4 aScProtectionAttr_Impl =
    {
     createSfxPoolItem<ScProtectionAttr>, &typeid(ScProtectionAttr), 4, { {MID_1,"Locked"}, {MID_2,"FormulasHidden"}, {MID_3,"Hidden"}, {MID_4,"HiddenInPrintout"} }
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
#ifdef ShellClass_TableSelection
#undef ShellClass
#undef ShellClass_TableSelection
#define ShellClass TableSelection
#endif

/************************************************************/
#ifdef ShellClass_TableDocument
#undef ShellClass
#undef ShellClass_TableDocument
#define ShellClass TableDocument
#endif

/************************************************************/
#ifdef ShellClass_ScDocShell
#undef ShellClass
#undef ShellClass_ScDocShell
#define ShellClass ScDocShell
static SfxFormalArgument aScDocShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "URL", SID_FILE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", SID_FILTER_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Password", SID_PASSWORD },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterOptions", SID_FILE_FILTEROPTIONS },
     { (const SfxType*) &aSfxInt16Item_Impl, "Version", SID_VERSION },
     { (const SfxType*) &aSfxBoolItem_Impl, "NoAcceptDialog", SID_NO_ACCEPT_DIALOG },
     { (const SfxType*) &aSfxStringItem_Impl, "URL", SID_FILE_NAME },
     { (const SfxType*) &aSfxInt16Item_Impl, "Version", SID_VERSION },
     { (const SfxType*) &aSfxStringItem_Impl, "File", SID_NOTEBOOKBAR },
     { (const SfxType*) &aSfxStringItem_Impl, "Language", SID_LANGUAGE_STATUS },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_CHART_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Range", SID_CHART_SOURCE },
     { (const SfxType*) &aSfxBoolItem_Impl, "ColHeaders", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "RowHeaders", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_CHART_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Range", SID_CHART_SOURCE },
     { (const SfxType*) &aSfxUnoAnyItem_Impl, "Query", SID_SBA_IMPORT },
     { (const SfxType*) &aSfxStringItem_Impl, "Target", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "Column", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt32Item_Impl, "Row", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "Table", FN_PARAM_3 },
     { (const SfxType*) &aSfxStringItem_Impl, "Text", SID_SC_SETTEXT },
     { (const SfxType*) &aSfxStringItem_Impl, "ScenarioName", SID_DELETE_SCENARIO },
     { (const SfxType*) &aSfxStringItem_Impl, "ScenarioName", SID_EDIT_SCENARIO }
    };

SFX_EXEC_STUB(ScDocShell,Execute)
SFX_STATE_STUB(ScDocShell,GetState)

static SfxSlot aScDocShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 6586
     SFX_NEW_SLOT_ARG( ScDocShell,SID_DOCUMENT_COMPARE,SfxGroupId::Edit,
                       &aScDocShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CompareDocuments" ),
     // Slot Nr. 1 : 6587
     SFX_NEW_SLOT_ARG( ScDocShell,SID_DOCUMENT_MERGE,SfxGroupId::Edit,
                       &aScDocShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MergeDocuments" ),
     // Slot Nr. 2 : 10022
     SFX_NEW_SLOT_ARG( ScDocShell,SID_ATTR_CHAR_FONTLIST,SfxGroupId::Document,
                       &aScDocShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontNameList" ),
     // Slot Nr. 3 : 10338
     SFX_NEW_SLOT_ARG( ScDocShell,SID_NOTEBOOKBAR,SfxGroupId::View,
                       &aScDocShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       8/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Notebookbar" ),
     // Slot Nr. 4 : 10441
     SFX_NEW_SLOT_ARG( ScDocShell,SID_GET_COLORLIST,SfxGroupId::Edit,
                       &aScDocShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       9/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GetColorTable" ),
     // Slot Nr. 5 : 11067
     SFX_NEW_SLOT_ARG( ScDocShell,SID_LANGUAGE_STATUS,SfxGroupId::Format,
                       &aScDocShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       9/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LanguageStatus" ),
     // Slot Nr. 6 : 12087
     SFX_NEW_SLOT_ARG( ScDocShell,SID_ATTR_YEAR2000,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Year2000" ),
     // Slot Nr. 7 : 26072
     SFX_NEW_SLOT_ARG( ScDocShell,SID_UPDATETABLINKS,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UpdateTableLinks" ),
     // Slot Nr. 8 : 26080
     SFX_NEW_SLOT_ARG( ScDocShell,SID_REIMPORT_AFTER_LOAD,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ReImportAfterLoad" ),
     // Slot Nr. 9 : 26084
     SFX_NEW_SLOT_ARG( ScDocShell,SID_CHG_PROTECT,SfxGroupId::Edit,
                       &aScDocShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ProtectTraceChangeMode" ),
     // Slot Nr. 10 : 26125
     SFX_NEW_SLOT_ARG( ScDocShell,SID_CHART_SOURCE,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeChartData" ),
     // Slot Nr. 11 : 26127
     SFX_NEW_SLOT_ARG( ScDocShell,SID_CHART_ADDSOURCE,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AddChartData" ),
     // Slot Nr. 12 : 26128
     SFX_NEW_SLOT_ARG( ScDocShell,SID_AUTO_STYLE,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AutoStyle" ),
     // Slot Nr. 13 : 26204
     SFX_NEW_SLOT_ARG( ScDocShell,SID_OPEN_CALC,SfxGroupId::Options,
                       &aScDocShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       16/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenFromCalc" ),
     // Slot Nr. 14 : 26238
     SFX_NEW_SLOT_ARG( ScDocShell,FID_CHG_RECORD,SfxGroupId::Edit,
                       &aScDocShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"TraceChangeMode" ),
     // Slot Nr. 15 : 26310
     SFX_NEW_SLOT_ARG( ScDocShell,FID_AUTO_CALC,SfxGroupId::Options,
                       &aScDocShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"AutomaticCalculation" ),
     // Slot Nr. 16 : 26311
     SFX_NEW_SLOT_ARG( ScDocShell,FID_RECALC,SfxGroupId::Options,
                       &aScDocShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Calculate" ),
     // Slot Nr. 17 : 26318
     SFX_NEW_SLOT_ARG( ScDocShell,SID_SHARE_DOC,SfxGroupId::Options,
                       &aScDocShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"ShareDocument" ),
     // Slot Nr. 18 : 26325
     SFX_NEW_SLOT_ARG( ScDocShell,FID_HARD_RECALC,SfxGroupId::Options,
                       &aScDocShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CalculateHard" ),
     // Slot Nr. 19 : 26349
     SFX_NEW_SLOT_ARG( ScDocShell,SID_SBA_IMPORT,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SbaImport" ),
     // Slot Nr. 20 : 26617
     SFX_NEW_SLOT_ARG( ScDocShell,SID_SC_SETTEXT,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       18/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PutCell" ),
     // Slot Nr. 21 : 26654
     SFX_NEW_SLOT_ARG( ScDocShell,SID_TABLES_COUNT,SfxGroupId::Document,
                       &aScDocShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDocShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       22/*Offset*/, 0, SfxSlotMode::NONE,"TableCount" ),
     // Slot Nr. 22 : 26666
     SFX_NEW_SLOT_ARG( ScDocShell,SID_DELETE_SCENARIO,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteScenario" ),
     // Slot Nr. 23 : 26667
     SFX_NEW_SLOT_ARG( ScDocShell,SID_EDIT_SCENARIO,SfxGroupId::Intern,
                       &aScDocShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScDocShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       23/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditScenario" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TableOLEObject
#undef ShellClass
#undef ShellClass_TableOLEObject
#define ShellClass TableOLEObject
#endif

/************************************************************/
#ifdef ShellClass_Tables
#undef ShellClass
#undef ShellClass_Tables
#define ShellClass Tables
#endif

/************************************************************/
#ifdef ShellClass_BaseSelection
#undef ShellClass
#undef ShellClass_BaseSelection
#define ShellClass BaseSelection
#endif

/************************************************************/
#ifdef ShellClass_TableEditView
#undef ShellClass
#undef ShellClass_TableEditView
#define ShellClass TableEditView
#endif

/************************************************************/
#ifdef ShellClass_ScTabViewShell
#undef ShellClass
#undef ShellClass_ScTabViewShell
#define ShellClass ScTabViewShell
static SfxFormalArgument aScTabViewShellArgs_Impl[] =
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
     { (const SfxType*) &aSfxGlobalNameItem_Impl, "ClassId", SID_INSERT_OBJECT },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "URL", FN_PARAM_2 },
     { (const SfxType*) &aSvxSizeItem_Impl, "Margin", FN_PARAM_3 },
     { (const SfxType*) &aSfxByteItem_Impl, "ScrollingMode", FN_PARAM_4 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsBorder", FN_PARAM_5 },
     { (const SfxType*) &aSfxStringItem_Impl, "Bookmark", SID_JUMPTOMARK },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartX", ID_VAL_MOUSESTART_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseStartY", ID_VAL_MOUSESTART_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndX", ID_VAL_MOUSEEND_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "MouseEndY", ID_VAL_MOUSEEND_Y },
     { (const SfxType*) &aSfxUInt16Item_Impl, "FillTransparence", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "FillColor", FN_PARAM_2 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "LineStyle", FN_PARAM_3 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsSticky", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "ShapeName", SID_SHAPE_NAME },
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
     { (const SfxType*) &aSfxBoolItem_Impl, "InNewTable", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "RangeList", FN_PARAM_5 },
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", SID_INSERT_GRAPHIC },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", FN_PARAM_FILTER },
     { (const SfxType*) &aSfxBoolItem_Impl, "AsLink", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Style", FN_PARAM_2 },
     { (const SfxType*) &aSvxGalleryItem_Impl, "GalleryItem", SID_GALLERY_FORMATS },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Transparence", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", FN_PARAM_2 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Width", FN_PARAM_3 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IsSticky", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "ShapeName", SID_SHAPE_NAME },
     { (const SfxType*) &aSvxSearchItem_Impl, "SearchItem", SID_SEARCH_ITEM },
     { (const SfxType*) &aSfxBoolItem_Impl, "Quiet", SID_SEARCH_QUIET },
     { (const SfxType*) &aSfxStringItem_Impl, "BasicShapes", SID_DRAWTBX_CS_BASIC },
     { (const SfxType*) &aSfxStringItem_Impl, "SymbolShapes", SID_DRAWTBX_CS_SYMBOL },
     { (const SfxType*) &aSfxStringItem_Impl, "ArrowShapes", SID_DRAWTBX_CS_ARROW },
     { (const SfxType*) &aSfxStringItem_Impl, "FlowChartShapes", SID_DRAWTBX_CS_FLOWCHART },
     { (const SfxType*) &aSfxStringItem_Impl, "CalloutShapes", SID_DRAWTBX_CS_CALLOUT },
     { (const SfxType*) &aSfxStringItem_Impl, "StarShapes", SID_DRAWTBX_CS_STAR },
     { (const SfxType*) &aSfxStringItem_Impl, "ToPoint", SID_CURRENTCELL },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Nr", SID_CURRENTTAB },
     { (const SfxType*) &aSfxStringItem_Impl, "DocName", SID_CURRENTDOC },
     { (const SfxType*) &aSfxStringItem_Impl, "ToObject", SID_CURRENTOBJECT },
     { (const SfxType*) &aSfxIntegerListItem_Impl, "Tables", SID_SELECT_TABLES },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Index", FID_DELETE_TABLE },
     { (const SfxType*) &aSfxStringItem_Impl, "Comment", FID_CHG_COMMENT },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_INS_TABLE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Index", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_INS_TABLE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Index", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "aTableName", FID_TABLE_HIDE },
     { (const SfxType*) &aSfxStringItem_Impl, "aTableName", FID_TABLE_SHOW },
     { (const SfxType*) &aSfxBoolItem_Impl, "Protect", FID_PROTECT_TABLE },
     { (const SfxType*) &aSfxBoolItem_Impl, "Protect", FID_PROTECT_DOC },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_TAB_RENAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_TAB_RENAME },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Index", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "DocName", FID_TAB_MOVE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Index", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Copy", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_TAB_APPEND },
     { (const SfxType*) &aSvxColorItem_Impl, "TabBgColor", FID_TAB_SET_TAB_BG_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "PrintArea", SID_CHANGE_PRINTAREA },
     { (const SfxType*) &aSfxStringItem_Impl, "PrintRepeatRow", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "PrintRepeatCol", FN_PARAM_3 },
     { (const SfxType*) &aSfxStringItem_Impl, "SearchString", FID_SEARCH },
     { (const SfxType*) &aSfxStringItem_Impl, "SearchString", FID_REPLACE },
     { (const SfxType*) &aSfxStringItem_Impl, "ReplaceString", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "SearchString", FID_SEARCH_ALL },
     { (const SfxType*) &aSfxStringItem_Impl, "SearchString", FID_REPLACE_ALL },
     { (const SfxType*) &aSfxStringItem_Impl, "ReplaceString", FN_PARAM_1 }
    };

SFX_EXEC_STUB(ScTabViewShell,Execute)
SFX_STATE_STUB(ScTabViewShell,GetState)
SFX_EXEC_STUB(ScTabViewShell,ExecuteSave)
SFX_STATE_STUB(ScTabViewShell,GetSaveState)
SFX_EXEC_STUB(ScTabViewShell,ExecDrawIns)
SFX_STATE_STUB(ScTabViewShell,GetDrawInsState)
SFX_EXEC_STUB(ScTabViewShell,ExecuteUndo)
SFX_STATE_STUB(ScTabViewShell,GetUndoState)
SFX_EXEC_STUB(ScTabViewShell,ExecChildWin)
SFX_EXEC_STUB(ScTabViewShell,ExecDraw)
SFX_STATE_STUB(ScTabViewShell,GetDrawState)
SFX_EXEC_STUB(ScTabViewShell,ExecGallery)
SFX_EXEC_STUB(ScTabViewShell,ExecSearch)
SFX_EXEC_STUB(ScTabViewShell,ExecImageMap)
SFX_STATE_STUB(ScTabViewShell,GetImageMapState)
SFX_EXEC_STUB(ScTabViewShell,ExecuteTable)
SFX_STATE_STUB(ScTabViewShell,GetStateTable)
SFX_EXEC_STUB(ScTabViewShell,ExecuteObject)
SFX_STATE_STUB(ScTabViewShell,GetObjectState)
SFX_EXEC_STUB(ScTabViewShell,ExecDrawOpt)
SFX_STATE_STUB(ScTabViewShell,GetDrawOptState)

static SfxSlot aScTabViewShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5325
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_PRINTPREVIEW,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"PrintPreview" ),
     // Slot Nr. 1 : 5502
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_SAVEASDOC,SfxGroupId::Document,
                       &aScTabViewShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteSave),SFX_STUB_PTR(ScTabViewShell,GetSaveState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 12/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveAs" ),
     // Slot Nr. 2 : 5505
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_SAVEDOC,SfxGroupId::Document,
                       &aScTabViewShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteSave),SFX_STUB_PTR(ScTabViewShell,GetSaveState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       12/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Save" ),
     // Slot Nr. 3 : 5561
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_OBJECT,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObject" ),
     // Slot Nr. 4 : 5563
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_FLOATINGFRAME,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 5/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObjectFloatingFrame" ),
     // Slot Nr. 5 : 5598
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_JUMPTOMARK,SfxGroupId::Navigator,
                       &aScTabViewShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToMark" ),
     // Slot Nr. 6 : 5700
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_REDO,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteUndo),SFX_STUB_PTR(ScTabViewShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       23/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 7 : 5701
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_UNDO,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteUndo),SFX_STUB_PTR(ScTabViewShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       25/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 8 : 5960
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_GALLERY,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecChildWin),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 0, SfxSlotMode::NONE,"Gallery" ),
     // Slot Nr. 9 : 6000
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OBJECTRESIZE,SfxGroupId::Document,
                       &aScTabViewShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InPlaceObjectResize" ),
     // Slot Nr. 10 : 6694
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_AVMEDIA_PLAYER,SfxGroupId::Application,
                       &aScTabViewShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecChildWin),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AVMediaPlayer" ),
     // Slot Nr. 11 : 6696
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_AVMEDIA,SfxGroupId::Application,
                       &aScTabViewShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertAVMedia" ),
     // Slot Nr. 12 : 10000
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_ATTR_ZOOM,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomItem,
                       27/*Offset*/, 0, SfxSlotMode::NONE,"Zoom" ),
     // Slot Nr. 13 : 10102
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_LINE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       27/*Offset*/, 0, SfxSlotMode::NONE,"Line" ),
     // Slot Nr. 14 : 10103
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_XLINE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       27/*Offset*/, 0, SfxSlotMode::NONE,"Line_Diagonal" ),
     // Slot Nr. 15 : 10104
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_RECT,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       27/*Offset*/, 9/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Rect" ),
     // Slot Nr. 16 : 10110
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_ELLIPSE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Ellipse" ),
     // Slot Nr. 17 : 10112
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_PIE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       40/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Pie" ),
     // Slot Nr. 18 : 10114
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_ARC,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       46/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Arc" ),
     // Slot Nr. 19 : 10115
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_CIRCLECUT,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       52/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CircleCut" ),
     // Slot Nr. 20 : 10117
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_POLYGON,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       58/*Offset*/, 0, SfxSlotMode::NONE,"Polygon" ),
     // Slot Nr. 21 : 10118
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_BEZIER_FILL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       58/*Offset*/, 0, SfxSlotMode::NONE,"BezierFill" ),
     // Slot Nr. 22 : 10128
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OBJECT_SELECT,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       58/*Offset*/, 0, SfxSlotMode::NONE,"SelectObject" ),
     // Slot Nr. 23 : 10140
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_DIAGRAM,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       58/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObjectChart" ),
     // Slot Nr. 24 : 10234
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OUTLINE_DELETEALL,SfxGroupId::Data,
                       &aScTabViewShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClearOutline" ),
     // Slot Nr. 25 : 10240
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_FRAME,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertTextFrame" ),
     // Slot Nr. 26 : 10241
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_GRAPHIC,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertGraphic" ),
     // Slot Nr. 27 : 10244
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_DRAW,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       64/*Offset*/, 0, SfxSlotMode::NONE,"InsertDraw" ),
     // Slot Nr. 28 : 10253
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_TEXT,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       64/*Offset*/, 0, SfxSlotMode::NONE,"DrawText" ),
     // Slot Nr. 29 : 10254
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_CAPTION,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       64/*Offset*/, 0, SfxSlotMode::NONE,"DrawCaption" ),
     // Slot Nr. 30 : 10280
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_GALLERY_FORMATS,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecGallery),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       64/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertGalleryPic" ),
     // Slot Nr. 31 : 10281
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_SEARCH_OPTIONS,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"SearchOptions" ),
     // Slot Nr. 32 : 10291
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_SEARCH_ITEM,SfxGroupId::NONE,
                       &aScTabViewShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSearchItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"SearchProperties" ),
     // Slot Nr. 33 : 10371
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_IMAP,SfxGroupId::Graphic,
                       &aScTabViewShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecImageMap),SFX_STUB_PTR(ScTabViewShell,GetImageMapState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"ImageMapDialog" ),
     // Slot Nr. 34 : 10374
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_IMAP_EXEC,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecImageMap),SFX_STUB_PTR(ScTabViewShell,GetImageMapState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"ImageMapExecute" ),
     // Slot Nr. 35 : 10394
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_XPOLYGON,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"Polygon_Diagonal" ),
     // Slot Nr. 36 : 10395
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_POLYGON_NOFILL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"Polygon_Unfilled" ),
     // Slot Nr. 37 : 10396
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_XPOLYGON_NOFILL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"Polygon_Diagonal_Unfilled" ),
     // Slot Nr. 38 : 10397
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_BEZIER_NOFILL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"Bezier_Unfilled" ),
     // Slot Nr. 39 : 10401
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_LINES,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"LineToolbox" ),
     // Slot Nr. 40 : 10463
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_FREELINE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 0, SfxSlotMode::NONE,"Freeline" ),
     // Slot Nr. 41 : 10464
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_FREELINE_NOFILL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       65/*Offset*/, 5/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Freeline_Unfilled" ),
     // Slot Nr. 42 : 10465
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_TEXT_MARQUEE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       70/*Offset*/, 0, SfxSlotMode::NONE,"Text_Marquee" ),
     // Slot Nr. 43 : 10502
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_SEARCH_NOW,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       70/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExecuteSearch" ),
     // Slot Nr. 44 : 10628
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_FM_CREATE_CONTROL,SfxGroupId::Controls,
                       &aScTabViewShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       72/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateControl" ),
     // Slot Nr. 45 : 10765
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_FM_CREATE_FIELDCONTROL,SfxGroupId::Controls,
                       &aScTabViewShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       72/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateFieldControl" ),
     // Slot Nr. 46 : 10860
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_CREATE_SW_DRAWVIEW,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       72/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateSWDrawView" ),
     // Slot Nr. 47 : 10905
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_TEXT_VERTICAL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       72/*Offset*/, 0, SfxSlotMode::NONE,"VerticalText" ),
     // Slot Nr. 48 : 10906
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_CAPTION_VERTICAL,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       72/*Offset*/, 0, SfxSlotMode::NONE,"VerticalCaption" ),
     // Slot Nr. 49 : 10923
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_GETUNDOSTRINGS,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       72/*Offset*/, 0, SfxSlotMode::NONE,"GetUndoStrings" ),
     // Slot Nr. 50 : 10924
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_GETREDOSTRINGS,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringListItem,
                       72/*Offset*/, 0, SfxSlotMode::NONE,"GetRedoStrings" ),
     // Slot Nr. 51 : 10930
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_READONLY_MODE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       72/*Offset*/, 0, SfxSlotMode::NONE,"ReadOnlyMode" ),
     // Slot Nr. 52 : 10977
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_FONTWORK_GALLERY_FLOATER,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       72/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkGalleryFloater" ),
     // Slot Nr. 53 : 11047
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_CS_BASIC,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       72/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BasicShapes" ),
     // Slot Nr. 54 : 11048
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_CS_SYMBOL,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       73/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SymbolShapes" ),
     // Slot Nr. 55 : 11049
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_CS_ARROW,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       74/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ArrowShapes" ),
     // Slot Nr. 56 : 11050
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_CS_FLOWCHART,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       75/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FlowChartShapes" ),
     // Slot Nr. 57 : 11051
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_CS_CALLOUT,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       76/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CalloutShapes" ),
     // Slot Nr. 58 : 11052
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_CS_STAR,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       77/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StarShapes" ),
     // Slot Nr. 59 : 11053
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_CS_ID,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"CustomShape" ),
     // Slot Nr. 60 : 11065
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_ATTR_ZOOMSLIDER,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomSliderItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"ZoomSlider" ),
     // Slot Nr. 61 : 11159
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_ARROW_CIRCLE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowCircle" ),
     // Slot Nr. 62 : 11160
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_ARROW_SQUARE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowSquare" ),
     // Slot Nr. 63 : 11161
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_CIRCLE_ARROW,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineCircleArrow" ),
     // Slot Nr. 64 : 11162
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_SQUARE_ARROW,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineSquareArrow" ),
     // Slot Nr. 65 : 11163
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_ARROWS,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineArrows" ),
     // Slot Nr. 66 : 11164
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAWTBX_ARROWS,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"ArrowsToolbox" ),
     // Slot Nr. 67 : 11165
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_ARROW_START,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowStart" ),
     // Slot Nr. 68 : 11166
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINE_ARROW_END,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"LineArrowEnd" ),
     // Slot Nr. 69 : 11167
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_MEASURELINE,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"MeasureLine" ),
     // Slot Nr. 70 : 11173
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_SIGNATURELINE,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSignatureLine" ),
     // Slot Nr. 71 : 11174
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_EDIT_SIGNATURELINE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditSignatureLine" ),
     // Slot Nr. 72 : 11175
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_SIGN_SIGNATURELINE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SignSignatureLine" ),
     // Slot Nr. 73 : 11191
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_QRCODE,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertQrCode" ),
     // Slot Nr. 74 : 11192
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_EDIT_QRCODE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditQrCode" ),
     // Slot Nr. 75 : 12094
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OPT_LOCALE_CHANGED,SfxGroupId::Special,
                       &aScTabViewShellSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"OptionsLocaleChanged" ),
     // Slot Nr. 76 : 25018
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_TOGGLE_GRID,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       78/*Offset*/, 0, SfxSlotMode::NONE,"ToggleSheetGrid" ),
     // Slot Nr. 77 : 25020
     SFX_NEW_SLOT_ARG( ScTabViewShell,WID_SIMPLE_REF,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SimpleReferenz" ),
     // Slot Nr. 78 : 26010
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_SCALESTATUS,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StatusScale" ),
     // Slot Nr. 79 : 26012
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_REPAINT,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Repaint" ),
     // Slot Nr. 80 : 26041
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_CURRENTCELL,SfxGroupId::Navigator,
                       &aScTabViewShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       78/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToCell" ),
     // Slot Nr. 81 : 26042
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_CURRENTTAB,SfxGroupId::Navigator,
                       &aScTabViewShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       79/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToTable" ),
     // Slot Nr. 82 : 26043
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_CURRENTDOC,SfxGroupId::Navigator,
                       &aScTabViewShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       80/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GotoDocument" ),
     // Slot Nr. 83 : 26044
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_CURRENTOBJECT,SfxGroupId::Navigator,
                       &aScTabViewShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       81/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToObject" ),
     // Slot Nr. 84 : 26060
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_LINKS,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditLinks" ),
     // Slot Nr. 85 : 26063
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_INSERT_SMATH,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawIns),SFX_STUB_PTR(ScTabViewShell,GetDrawInsState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertObjectStarMath" ),
     // Slot Nr. 86 : 26069
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_WINDOW_SPLIT,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SplitWindow" ),
     // Slot Nr. 87 : 26070
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_WINDOW_FIX,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FreezePanes" ),
     // Slot Nr. 88 : 26071
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_CHART,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DrawChart" ),
     // Slot Nr. 89 : 26081
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DRAW_NOTEEDIT,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDraw),SFX_STUB_PTR(ScTabViewShell,GetDrawState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DrawEditNote" ),
     // Slot Nr. 90 : 26090
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_SELECT_TABLES,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       82/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectTables" ),
     // Slot Nr. 91 : 26115
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_STATUS_PAGESTYLE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       83/*Offset*/, 0, SfxSlotMode::NONE,"StatusPageStyle" ),
     // Slot Nr. 92 : 26184
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_WINDOW_FIX_ROW,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FreezePanesFirstRow" ),
     // Slot Nr. 93 : 26185
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_WINDOW_FIX_COL,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[127] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FreezePanesFirstColumn" ),
     // Slot Nr. 94 : 26223
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_DELETE_TABLE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[107] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       83/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Remove" ),
     // Slot Nr. 95 : 26235
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_HFEDIT,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       84/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditHeaderAndFooter" ),
     // Slot Nr. 96 : 26239
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_CHG_SHOW,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       84/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowChanges" ),
     // Slot Nr. 97 : 26241
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TOGGLEINPUTLINE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"InputLineVisible" ),
     // Slot Nr. 98 : 26242
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TOGGLEHEADERS,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"ViewRowColumnHeaders" ),
     // Slot Nr. 99 : 26244
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_SCALE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       84/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Scale" ),
     // Slot Nr. 100 : 26245
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TOGGLESYNTAX,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[101] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"ViewValueHighlighting" ),
     // Slot Nr. 101 : 26247
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_PAGEBREAKMODE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[102] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"PagebreakMode" ),
     // Slot Nr. 102 : 26248
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_FUNCTION_BOX,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[103] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"FunctionBox" ),
     // Slot Nr. 103 : 26249
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_NORMALVIEWMODE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[104] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"NormalViewMode" ),
     // Slot Nr. 104 : 26250
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TOGGLEFORMULA,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[105] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       84/*Offset*/, 0, SfxSlotMode::NONE,"ToggleFormula" ),
     // Slot Nr. 105 : 26258
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_CHG_ACCEPT,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[106] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       84/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AcceptChanges" ),
     // Slot Nr. 106 : 26259
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_CHG_COMMENT,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[111] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       84/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommentChange" ),
     // Slot Nr. 107 : 26269
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_INS_TABLE,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[108] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       85/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Insert" ),
     // Slot Nr. 108 : 26275
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_INS_TABLE_EXT,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[109] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       87/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSheetFromFile" ),
     // Slot Nr. 109 : 26294
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TABLE_HIDE,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[110] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       89/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Hide" ),
     // Slot Nr. 110 : 26295
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TABLE_SHOW,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[112] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       90/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Show" ),
     // Slot Nr. 111 : 26302
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_FORMATPAGE,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[113] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PageFormatDialog" ),
     // Slot Nr. 112 : 26305
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TABLE_VISIBLE,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[116] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       91/*Offset*/, 0, SfxSlotMode::NONE,"Visible" ),
     // Slot Nr. 113 : 26313
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_PROTECT_TABLE,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[114] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Protect" ),
     // Slot Nr. 114 : 26314
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_PROTECT_DOC,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[115] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       92/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToolProtectionDocument" ),
     // Slot Nr. 115 : 26341
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_AUTO_OUTLINE,SfxGroupId::Data,
                       &aScTabViewShellSlots_Impl[119] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       93/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AutoOutline" ),
     // Slot Nr. 116 : 26356
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_MENU_RENAME,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[117] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       93/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenameTable" ),
     // Slot Nr. 117 : 26357
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_RENAME,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[118] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       94/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Name" ),
     // Slot Nr. 118 : 26358
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_MOVE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[120] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       96/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Move" ),
     // Slot Nr. 119 : 26359
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_SELECTALL,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[122] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       99/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableSelectAll" ),
     // Slot Nr. 120 : 26360
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_APPEND,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[121] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       99/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Add" ),
     // Slot Nr. 121 : 26362
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_RTL,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[123] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       100/*Offset*/, 0, SfxSlotMode::NONE,"SheetRightToLeft" ),
     // Slot Nr. 122 : 26363
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_DESELECTALL,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[126] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       100/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableDeselectAll" ),
     // Slot Nr. 123 : 26364
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_MENU_SET_TAB_BG_COLOR,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[124] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       100/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetTabBgColor" ),
     // Slot Nr. 124 : 26365
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_SET_TAB_BG_COLOR,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[125] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       101/*Offset*/, 0, SfxSlotMode::NONE,"TabBgColor" ),
     // Slot Nr. 125 : 26366
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_TAB_EVENTS,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteTable),SFX_STUB_PTR(ScTabViewShell,GetStateTable),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableEvents" ),
     // Slot Nr. 126 : 26472
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DETECTIVE_DEL_ALL,SfxGroupId::Options,
                       &aScTabViewShellSlots_Impl[128] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClearArrows" ),
     // Slot Nr. 127 : 26598
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_TABLE_ACTIVATE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[130] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ActivateTable" ),
     // Slot Nr. 128 : 26602
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DEFINE_PRINTAREA,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[129] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefinePrintArea" ),
     // Slot Nr. 129 : 26603
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_DELETE_PRINTAREA,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[131] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeletePrintArea" ),
     // Slot Nr. 130 : 26604
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_CHANGE_PRINTAREA,SfxGroupId::Drawing,
                       &aScTabViewShellSlots_Impl[132] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangePrintArea" ),
     // Slot Nr. 131 : 26605
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OPENDLG_EDIT_PRINTAREA,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[140] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       104/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditPrintArea" ),
     // Slot Nr. 132 : 26607
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OLE_ACTIVATE,SfxGroupId::Special,
                       &aScTabViewShellSlots_Impl[133] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       104/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ActivateOLE" ),
     // Slot Nr. 133 : 26608
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OLE_DEACTIVATE,SfxGroupId::Special,
                       &aScTabViewShellSlots_Impl[134] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       104/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Deactivate" ),
     // Slot Nr. 134 : 26611
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_SEARCH,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[135] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       104/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Search" ),
     // Slot Nr. 135 : 26612
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_REPEAT_SEARCH,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[136] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RepeatSearch" ),
     // Slot Nr. 136 : 26613
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_REPLACE,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[137] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Replace" ),
     // Slot Nr. 137 : 26614
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_SEARCH_ALL,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[138] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       107/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SearchAll" ),
     // Slot Nr. 138 : 26615
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_REPLACE_ALL,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[139] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecSearch),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       108/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ReplaceAll" ),
     // Slot Nr. 139 : 26621
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OLE_SELECT,SfxGroupId::Special,
                       &aScTabViewShellSlots_Impl[141] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectOLE" ),
     // Slot Nr. 140 : 26624
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_INSERT_FILE,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[143] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFile" ),
     // Slot Nr. 141 : 26626
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_TBXCTL_INSERT,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[142] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"InsertCtrl" ),
     // Slot Nr. 142 : 26627
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_TBXCTL_INSCELLS,SfxGroupId::Insert,
                       &aScTabViewShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"InsCellsCtrl" ),
     // Slot Nr. 143 : 26650
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_DEL_MANUALBREAKS,SfxGroupId::Edit,
                       &aScTabViewShellSlots_Impl[144] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteAllBreaks" ),
     // Slot Nr. 144 : 26651
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_ADD_PRINTAREA,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[145] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AddPrintArea" ),
     // Slot Nr. 145 : 26652
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_ADJUST_PRINTZOOM,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[146] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AdjustPrintZoom" ),
     // Slot Nr. 146 : 26653
     SFX_NEW_SLOT_ARG( ScTabViewShell,FID_RESET_PRINTZOOM,SfxGroupId::Format,
                       &aScTabViewShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,Execute),SFX_STUB_PTR(ScTabViewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       110/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ResetPrintZoom" ),
     // Slot Nr. 147 : 26655
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_ACTIVE_OBJ_NAME,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[148] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScTabViewShell,GetObjectState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"SelectedObjectName" ),
     // Slot Nr. 148 : 26656
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OBJECT_LEFT,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[149] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR(ScTabViewShell,GetObjectState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"ObjectLeft" ),
     // Slot Nr. 149 : 26657
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OBJECT_TOP,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[150] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR(ScTabViewShell,GetObjectState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"ObjectTop" ),
     // Slot Nr. 150 : 26658
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OBJECT_WIDTH,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[151] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR(ScTabViewShell,GetObjectState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"ObjectWidth" ),
     // Slot Nr. 151 : 26659
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_OBJECT_HEIGHT,SfxGroupId::Intern,
                       &aScTabViewShellSlots_Impl[147] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecuteObject),SFX_STUB_PTR(ScTabViewShell,GetObjectState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"ObjectHeight" ),
     // Slot Nr. 152 : 27153
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_HELPLINES_MOVE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[153] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawOpt),SFX_STUB_PTR(ScTabViewShell,GetDrawOptState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"HelplinesMove" ),
     // Slot Nr. 153 : 27154
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_GRID_USE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[154] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawOpt),SFX_STUB_PTR(ScTabViewShell,GetDrawOptState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"GridUse" ),
     // Slot Nr. 154 : 27322
     SFX_NEW_SLOT_ARG( ScTabViewShell,SID_GRID_VISIBLE,SfxGroupId::View,
                       &aScTabViewShellSlots_Impl[152] /*Offset Next*/,
                       SFX_STUB_PTR(ScTabViewShell,ExecDrawOpt),SFX_STUB_PTR(ScTabViewShell,GetDrawOptState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       110/*Offset*/, 0, SfxSlotMode::NONE,"GridVisible" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TableDraw
#undef ShellClass
#undef ShellClass_TableDraw
#define ShellClass TableDraw
#endif

/************************************************************/
#ifdef ShellClass_ScDrawShell
#undef ShellClass
#undef ShellClass_ScDrawShell
#define ShellClass ScDrawShell
static SfxFormalArgument aScDrawShellArgs_Impl[] =
    {
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
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "PersistentCopy", SID_FORMATPAINTBRUSH },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_HIDE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_SHOW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformPosX", SID_ATTR_TRANSFORM_POS_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformPosY", SID_ATTR_TRANSFORM_POS_Y },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformWidth", SID_ATTR_TRANSFORM_WIDTH },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformHeight", SID_ATTR_TRANSFORM_HEIGHT },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationDeltaAngle", SID_ATTR_TRANSFORM_DELTA_ANGLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationAngle", SID_ATTR_TRANSFORM_ANGLE },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationX", SID_ATTR_TRANSFORM_ROT_X },
     { (const SfxType*) &aSfxUInt32Item_Impl, "TransformRotationY", SID_ATTR_TRANSFORM_ROT_Y },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aXFillColorItem_Impl, "FillColor", SID_ATTR_FILL_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "FillGradientJSON", SID_FILL_GRADIENT_JSON },
     { (const SfxType*) &aSvxDoubleItem_Impl, "Width", SID_ATTR_LINE_WIDTH_ARG },
     { (const SfxType*) &aXLineWidthItem_Impl, "LineWidth", SID_ATTR_LINE_WIDTH },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aXLineColorItem_Impl, "XLineColor", SID_ATTR_LINE_COLOR },
     { (const SfxType*) &aXLineStartItem_Impl, "LineStart", SID_ATTR_LINE_START },
     { (const SfxType*) &aXLineEndItem_Impl, "LineEnd", SID_ATTR_LINE_END },
     { (const SfxType*) &aSfxUInt32Item_Impl, "StartWidth", SID_ATTR_LINE_STARTWIDTH },
     { (const SfxType*) &aSfxUInt32Item_Impl, "EndWidth", SID_ATTR_LINE_ENDWIDTH },
     { (const SfxType*) &aSvxHyperlinkItem_Impl, "Hyperlink", SID_HYPERLINK_SETLINK },
     { (const SfxType*) &aSvxDoubleItem_Impl, "Depth", SID_EXTRUSION_DEPTH },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Metric", SID_ATTR_METRIC },
     { (const SfxType*) &aSfxStringItem_Impl, "FontworkShapeType", SID_FONTWORK_SHAPE_TYPE },
     { (const SfxType*) &aSfxInt32Item_Impl, "FontworkCharacterSpacing", SID_FONTWORK_CHARACTER_SPACING },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_DEFINE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbol", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Options", FN_PARAM_2 }
    };

SFX_STATE_STUB(ScDrawShell,StateDisableItems)
SFX_EXEC_STUB(ScDrawShell,ExecDrawFunc)
SFX_STATE_STUB(ScDrawShell,GetDrawFuncState)
SFX_EXEC_STUB(ScDrawShell,ExecFormatPaintbrush)
SFX_STATE_STUB(ScDrawShell,StateFormatPaintbrush)
SFX_EXEC_STUB(ScDrawShell,ExecDrawAttr)
SFX_STATE_STUB(ScDrawShell,GetDrawAttrStateForIFBX)
SFX_STATE_STUB(ScDrawShell,GetState)
SFX_STATE_STUB(ScDrawShell,GetAttrFuncState)
SFX_STATE_STUB(ScDrawShell,GetDrawAttrState)
SFX_EXEC_STUB(ScDrawShell,ExecFormText)
SFX_STATE_STUB(ScDrawShell,GetFormTextState)
SFX_STATE_STUB(ScDrawShell,GetHLinkState)
SFX_EXEC_STUB(ScDrawShell,ExecuteHLink)

static SfxSlot aScDrawShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5542
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_FAMILY2,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"ParaStyle" ),
     // Slot Nr. 1 : 5544
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_FAMILY4,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"PageStyle" ),
     // Slot Nr. 2 : 5549
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_NEW,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewStyle" ),
     // Slot Nr. 3 : 5550
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_EDIT,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditStyle" ),
     // Slot Nr. 4 : 5551
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_DELETE,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteStyle" ),
     // Slot Nr. 5 : 5552
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_APPLY,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       6/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleApply" ),
     // Slot Nr. 6 : 5554
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_WATERCAN,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"StyleWatercanMode" ),
     // Slot Nr. 7 : 5555
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_NEW_BY_EXAMPLE,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleNewByExample" ),
     // Slot Nr. 8 : 5556
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_UPDATE_BY_EXAMPLE,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       12/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleUpdateByExample" ),
     // Slot Nr. 9 : 5710
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_CUT,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 10 : 5711
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_COPY,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 11 : 5712
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_PASTE,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 12 : 5713
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DELETE,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 13 : 5715
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMATPAINTBRUSH,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormatPaintbrush),SFX_STUB_PTR(ScDrawShell,StateFormatPaintbrush),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       16/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatPaintbrush" ),
     // Slot Nr. 14 : 5723
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_SELECTALL,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 15 : 6603
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_HIDE,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideStyle" ),
     // Slot Nr. 16 : 6604
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_STYLE_SHOW,SfxGroupId::Template,
                       &aScDrawShellSlots_Impl[135] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowStyle" ),
     // Slot Nr. 17 : 10087
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 8/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TransformDialog" ),
     // Slot Nr. 18 : 10088
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_POS_X,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformPosX" ),
     // Slot Nr. 19 : 10089
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_POS_Y,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformPosY" ),
     // Slot Nr. 20 : 10090
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_WIDTH,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformWidth" ),
     // Slot Nr. 21 : 10091
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_HEIGHT,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformHeight" ),
     // Slot Nr. 22 : 10093
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_ROT_X,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformRotationX" ),
     // Slot Nr. 23 : 10094
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_ROT_Y,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformRotationY" ),
     // Slot Nr. 24 : 10095
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_ANGLE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TransformRotationAngle" ),
     // Slot Nr. 25 : 10126
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_BEZIER_EDIT,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleObjectBezierMode" ),
     // Slot Nr. 26 : 10129
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ROTATE,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleObjectRotateMode" ),
     // Slot Nr. 27 : 10130
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"ObjectAlign" ),
     // Slot Nr. 28 : 10131
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN_LEFT,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectAlignLeft" ),
     // Slot Nr. 29 : 10132
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN_CENTER,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignCenter" ),
     // Slot Nr. 30 : 10133
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN_RIGHT,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectAlignRight" ),
     // Slot Nr. 31 : 10134
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN_UP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignUp" ),
     // Slot Nr. 32 : 10135
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN_MIDDLE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignMiddle" ),
     // Slot Nr. 33 : 10136
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_ALIGN_DOWN,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignDown" ),
     // Slot Nr. 34 : 10142
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTRIBUTES_AREA,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetAttrFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatArea" ),
     // Slot Nr. 35 : 10143
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTRIBUTES_LINE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetAttrFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatLine" ),
     // Slot Nr. 36 : 10164
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_STYLE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillStyleItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"FillStyle" ),
     // Slot Nr. 37 : 10165
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_COLOR,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillColorItem,
                       29/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillColor" ),
     // Slot Nr. 38 : 10166
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_GRADIENT,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillGradientItem,
                       31/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillGradient" ),
     // Slot Nr. 39 : 10167
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_HATCH,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillHatchItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"FillHatch" ),
     // Slot Nr. 40 : 10168
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_BITMAP,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillBitmapItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"FillBitmap" ),
     // Slot Nr. 41 : 10169
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_STYLE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineStyleItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"XLineStyle" ),
     // Slot Nr. 42 : 10170
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_DASH,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineDashItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"LineDash" ),
     // Slot Nr. 43 : 10171
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_WIDTH,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineWidthItem,
                       32/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineWidth" ),
     // Slot Nr. 44 : 10172
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_COLOR,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineColorItem,
                       34/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"XLineColor" ),
     // Slot Nr. 45 : 10173
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_START,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineStartItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"LineStart" ),
     // Slot Nr. 46 : 10174
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_END,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineEndItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"LineEnd" ),
     // Slot Nr. 47 : 10179
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_COLOR_TABLE,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"ColorTableState" ),
     // Slot Nr. 48 : 10180
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_GRADIENT_LIST,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxGradientListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"GradientListState" ),
     // Slot Nr. 49 : 10181
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_HATCH_LIST,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHatchListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"HatchListState" ),
     // Slot Nr. 50 : 10182
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_BITMAP_LIST,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBitmapListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"BitmapListState" ),
     // Slot Nr. 51 : 10183
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_PATTERN_LIST,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPatternListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"PatternListState" ),
     // Slot Nr. 52 : 10184
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DASH_LIST,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxDashListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"DashListState" ),
     // Slot Nr. 53 : 10185
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_LINEEND_LIST,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineEndListItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"LineEndListState" ),
     // Slot Nr. 54 : 10223
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_POSITION,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxPointItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"Position" ),
     // Slot Nr. 55 : 10224
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_SIZE,SfxGroupId::View,
                       &aScDrawShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSizeItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"Size" ),
     // Slot Nr. 56 : 10225
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_TABLE_CELL,SfxGroupId::View,
                       &aScDrawShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"StateTableCell" ),
     // Slot Nr. 57 : 10236
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_PROTECT_POS,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"ProtectPos" ),
     // Slot Nr. 58 : 10237
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_PROTECT_SIZE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"ProtectSize" ),
     // Slot Nr. 59 : 10256
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[117] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWork" ),
     // Slot Nr. 60 : 10257
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_STYLE,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextStyleItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextStyle" ),
     // Slot Nr. 61 : 10258
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_ADJUST,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextAdjustItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextAdjust" ),
     // Slot Nr. 62 : 10259
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_DISTANCE,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextDistanceItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextDistance" ),
     // Slot Nr. 63 : 10260
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_START,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextStartItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextStart" ),
     // Slot Nr. 64 : 10261
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_MIRROR,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextMirrorItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextMirror" ),
     // Slot Nr. 65 : 10262
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_OUTLINE,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextOutlineItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextOutline" ),
     // Slot Nr. 66 : 10263
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_SHADOW,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadow" ),
     // Slot Nr. 67 : 10264
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_SHDWCOLOR,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowColorItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowColor" ),
     // Slot Nr. 68 : 10265
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_SHDWXVAL,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowXValItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowXVal" ),
     // Slot Nr. 69 : 10266
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_SHDWYVAL,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowYValItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowYVal" ),
     // Slot Nr. 70 : 10268
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FORMTEXT_HIDEFORM,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecFormText),SFX_STUB_PTR(ScDrawShell,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextHideFormItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FormatFontWorkClose" ),
     // Slot Nr. 71 : 10282
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_HELL,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetObjectToBackground" ),
     // Slot Nr. 72 : 10283
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_HEAVEN,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetObjectToForeground" ),
     // Slot Nr. 73 : 10286
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FRAME_TO_TOP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BringToFront" ),
     // Slot Nr. 74 : 10287
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FRAME_TO_BOTTOM,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       36/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SendToBack" ),
     // Slot Nr. 75 : 10299
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_SHADOW,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[120] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrOnOffItem,
                       36/*Offset*/, 0, SfxSlotMode::NONE,"FillShadow" ),
     // Slot Nr. 76 : 10301
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINEEND_STYLE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[132] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetAttrFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       36/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LineEndStyle" ),
     // Slot Nr. 77 : 10310
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_AUTOWIDTH,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"AutomaticWidth" ),
     // Slot Nr. 78 : 10311
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_TRANSFORM_AUTOHEIGHT,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawAttrStateForIFBX),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"AutomaticHeight" ),
     // Slot Nr. 79 : 10361
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_HYPERLINK_GETLINK,SfxGroupId::Insert,
                       &aScDrawShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetHLinkState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHyperlinkItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"Hyperlink" ),
     // Slot Nr. 80 : 10362
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_HYPERLINK_SETLINK,SfxGroupId::Insert,
                       &aScDrawShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecuteHLink),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetHyperlink" ),
     // Slot Nr. 81 : 10454
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_GROUP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatGroup" ),
     // Slot Nr. 82 : 10455
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_UNGROUP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatUngroup" ),
     // Slot Nr. 83 : 10955
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPEN_HYPERLINK,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenHyperlinkOnCursor" ),
     // Slot Nr. 84 : 10958
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPEN_XML_FILTERSETTINGS,SfxGroupId::Options,
                       &aScDrawShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenXMLFilterSettings" ),
     // Slot Nr. 85 : 10960
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_TOGGLE,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionToggle" ),
     // Slot Nr. 86 : 10961
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_TILT_DOWN,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltDown" ),
     // Slot Nr. 87 : 10962
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_TILT_UP,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltUp" ),
     // Slot Nr. 88 : 10963
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_TILT_LEFT,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltLeft" ),
     // Slot Nr. 89 : 10964
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_TILT_RIGHT,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionTiltRight" ),
     // Slot Nr. 90 : 10965
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_DEPTH_FLOATER,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionDepthFloater" ),
     // Slot Nr. 91 : 10966
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_DIRECTION_FLOATER,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionDirectionFloater" ),
     // Slot Nr. 92 : 10967
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_LIGHTING_FLOATER,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionLightingFloater" ),
     // Slot Nr. 93 : 10968
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_SURFACE_FLOATER,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionSurfaceFloater" ),
     // Slot Nr. 94 : 10969
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_3D_COLOR,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"Extrusion3DColor" ),
     // Slot Nr. 95 : 10970
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_DEPTH,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxDoubleItem,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionDepth" ),
     // Slot Nr. 96 : 10971
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_DIRECTION,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionDirection" ),
     // Slot Nr. 97 : 10972
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_PROJECTION,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionProjection" ),
     // Slot Nr. 98 : 10973
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_LIGHTING_DIRECTION,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionLightingDirection" ),
     // Slot Nr. 99 : 10974
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_LIGHTING_INTENSITY,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionLightingIntensity" ),
     // Slot Nr. 100 : 10975
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_SURFACE,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[101] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       41/*Offset*/, 0, SfxSlotMode::NONE,"ExtrusionSurface" ),
     // Slot Nr. 101 : 10976
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_EXTRUSION_DEPTH_DIALOG,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[102] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxDoubleItem,
                       41/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExtrusionDepthDialog" ),
     // Slot Nr. 102 : 10979
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_SHAPE_TYPE,SfxGroupId::Options,
                       &aScDrawShellSlots_Impl[103] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       43/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkShapeType" ),
     // Slot Nr. 103 : 10980
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_SAME_LETTER_HEIGHTS,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[104] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       44/*Offset*/, 0, SfxSlotMode::NONE,"FontworkSameLetterHeights" ),
     // Slot Nr. 104 : 10981
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_ALIGNMENT_FLOATER,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[105] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkAlignmentFloater" ),
     // Slot Nr. 105 : 10982
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_CHARACTER_SPACING_FLOATER,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[106] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkCharacterSpacingFloater" ),
     // Slot Nr. 106 : 10983
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_SHAPE,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[107] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       44/*Offset*/, 0, SfxSlotMode::NONE,"FontworkShape" ),
     // Slot Nr. 107 : 10984
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_ALIGNMENT,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[108] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       44/*Offset*/, 0, SfxSlotMode::NONE,"FontworkAlignment" ),
     // Slot Nr. 108 : 10985
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_CHARACTER_SPACING,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[109] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       44/*Offset*/, 0, SfxSlotMode::NONE,"FontworkCharacterSpacing" ),
     // Slot Nr. 109 : 10986
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_KERN_CHARACTER_PAIRS,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[110] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       44/*Offset*/, 0, SfxSlotMode::NONE,"FontworkKernCharacterPairs" ),
     // Slot Nr. 110 : 11002
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ALIGN_ANY_LEFT,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[111] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignLeft" ),
     // Slot Nr. 111 : 11003
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ALIGN_ANY_HCENTER,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[112] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignHorizontalCenter" ),
     // Slot Nr. 112 : 11004
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ALIGN_ANY_RIGHT,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[113] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignRight" ),
     // Slot Nr. 113 : 11006
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ALIGN_ANY_TOP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[114] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignTop" ),
     // Slot Nr. 114 : 11007
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ALIGN_ANY_VCENTER,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[115] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignVerticalCenter" ),
     // Slot Nr. 115 : 11008
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ALIGN_ANY_BOTTOM,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[116] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignBottom" ),
     // Slot Nr. 116 : 11054
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FONTWORK_CHARACTER_SPACING_DIALOG,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[118] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       44/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontworkCharacterSpacingDialog" ),
     // Slot Nr. 117 : 11093
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OBJECT_MIRROR,SfxGroupId::Special,
                       &aScDrawShellSlots_Impl[154] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Mirror" ),
     // Slot Nr. 118 : 11118
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FRAME_UP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[119] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectForwardOne" ),
     // Slot Nr. 119 : 11119
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FRAME_DOWN,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[123] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectBackOne" ),
     // Slot Nr. 120 : 11124
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_TRANSPARENCE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[121] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FillTransparence" ),
     // Slot Nr. 121 : 11125
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_FILL_FLOATTRANSPARENCE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[122] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFillFloatTransparenceItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FillFloatTransparence" ),
     // Slot Nr. 122 : 11126
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_TRANSPARENCE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[125] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"LineTransparence" ),
     // Slot Nr. 123 : 11127
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FLIP_HORIZONTAL,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[124] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FlipHorizontal" ),
     // Slot Nr. 124 : 11128
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FLIP_VERTICAL,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[131] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FlipVertical" ),
     // Slot Nr. 125 : 11129
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_JOINT,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[126] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineJointItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"LineJoint" ),
     // Slot Nr. 126 : 11130
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_LINE_CAP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[127] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XLineCapItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"LineCap" ),
     // Slot Nr. 127 : 11148
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_SHADOW_TRANSPARENCE,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[128] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrPercentItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowTransparency" ),
     // Slot Nr. 128 : 11149
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_SHADOW_COLOR,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[129] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XColorItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowColor" ),
     // Slot Nr. 129 : 11150
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_SHADOW_XDISTANCE,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[130] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrMetricItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowXDistance" ),
     // Slot Nr. 130 : 11151
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ATTR_SHADOW_YDISTANCE,SfxGroupId::Document,
                       &aScDrawShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SdrMetricItem,
                       45/*Offset*/, 0, SfxSlotMode::NONE,"FillShadowYDistance" ),
     // Slot Nr. 131 : 11172
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ANCHOR_MENU,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[133] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AnchorMenu" ),
     // Slot Nr. 132 : 11176
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_MEASURE_DLG,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetAttrFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MeasureAttributes" ),
     // Slot Nr. 133 : 26065
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_MIRROR_VERTICAL,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[134] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectMirrorVertical" ),
     // Slot Nr. 134 : 26066
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_MIRROR_HORIZONTAL,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[136] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectMirrorHorizontal" ),
     // Slot Nr. 135 : 26071
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DRAW_CHART,SfxGroupId::Insert,
                       &aScDrawShellSlots_Impl[142] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DrawChart" ),
     // Slot Nr. 136 : 26087
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ENABLE_HYPHENATION,SfxGroupId::Options,
                       &aScDrawShellSlots_Impl[137] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Hyphenate" ),
     // Slot Nr. 137 : 26088
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_RENAME_OBJECT,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[138] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RenameObject" ),
     // Slot Nr. 138 : 26094
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_TITLE_DESCRIPTION_OBJECT,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[140] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ObjectTitleDescription" ),
     // Slot Nr. 139 : 26095
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ASSIGNMACRO,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[149] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AssignMacro" ),
     // Slot Nr. 140 : 26096
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DRAW_HLINK_EDIT,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[141] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditShapeHyperlink" ),
     // Slot Nr. 141 : 26097
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DRAW_HLINK_DELETE,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[157] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteShapeHyperlink" ),
     // Slot Nr. 142 : 26150
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPENDLG_CONSOLIDATE,SfxGroupId::Data,
                       &aScDrawShellSlots_Impl[143] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataConsolidate" ),
     // Slot Nr. 143 : 26151
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPENDLG_PIVOTTABLE,SfxGroupId::Data,
                       &aScDrawShellSlots_Impl[144] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataDataPilotRun" ),
     // Slot Nr. 144 : 26153
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPENDLG_SOLVE,SfxGroupId::Options,
                       &aScDrawShellSlots_Impl[145] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoalSeekDialog" ),
     // Slot Nr. 145 : 26154
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPENDLG_TABOP,SfxGroupId::Data,
                       &aScDrawShellSlots_Impl[146] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableOperationDialog" ),
     // Slot Nr. 146 : 26160
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPENDLG_OPTSOLVER,SfxGroupId::Options,
                       &aScDrawShellSlots_Impl[147] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SolverDialog" ),
     // Slot Nr. 147 : 26271
     SFX_NEW_SLOT_ARG( ScDrawShell,FID_DEFINE_NAME,SfxGroupId::Insert,
                       &aScDrawShellSlots_Impl[148] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       45/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineName" ),
     // Slot Nr. 148 : 26276
     SFX_NEW_SLOT_ARG( ScDrawShell,FID_ADD_NAME,SfxGroupId::Insert,
                       &aScDrawShellSlots_Impl[151] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AddName" ),
     // Slot Nr. 149 : 26303
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_TEXT_STANDARD,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[150] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StandardTextAttributes" ),
     // Slot Nr. 150 : 26304
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DRAWTEXT_ATTR_DLG,SfxGroupId::Drawing,
                       &aScDrawShellSlots_Impl[139] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawAttr),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TextAttributes" ),
     // Slot Nr. 151 : 26327
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DEFINE_DBNAME,SfxGroupId::Data,
                       &aScDrawShellSlots_Impl[152] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineDBName" ),
     // Slot Nr. 152 : 26330
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FILTER,SfxGroupId::Data,
                       &aScDrawShellSlots_Impl[153] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterStandardFilter" ),
     // Slot Nr. 153 : 26331
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_SPECIAL_FILTER,SfxGroupId::Data,
                       &aScDrawShellSlots_Impl[161] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterSpecialFilter" ),
     // Slot Nr. 154 : 26421
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ANCHOR_PAGE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[155] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetAnchorToPage" ),
     // Slot Nr. 155 : 26422
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ANCHOR_CELL,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[156] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetAnchorToCell" ),
     // Slot Nr. 156 : 26423
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ANCHOR_CELL_RESIZE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetAnchorToCellResize" ),
     // Slot Nr. 157 : 26424
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ANCHOR_TOGGLE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[158] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleAnchorType" ),
     // Slot Nr. 158 : 26425
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ORIGINALSIZE,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[159] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OriginalSize" ),
     // Slot Nr. 159 : 26426
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_FITCELLSIZE,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[160] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FitCellSize" ),
     // Slot Nr. 160 : 26553
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DELETE_CONTENTS,SfxGroupId::Edit,
                       &aScDrawShellSlots_Impl[163] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClearContents" ),
     // Slot Nr. 161 : 26605
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_OPENDLG_EDIT_PRINTAREA,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[162] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditPrintArea" ),
     // Slot Nr. 162 : 26629
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_DEFINE_COLROWNAMERANGES,SfxGroupId::Insert,
                       &aScDrawShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawShell,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineLabelRange" ),
     // Slot Nr. 163 : 27096
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_ENTER_GROUP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[164] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EnterGroup" ),
     // Slot Nr. 164 : 27097
     SFX_NEW_SLOT_ARG( ScDrawShell,SID_LEAVE_GROUP,SfxGroupId::Format,
                       &aScDrawShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawShell,ExecDrawFunc),SFX_STUB_PTR(ScDrawShell,GetDrawFuncState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       48/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LeaveGroup" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TableText
#undef ShellClass
#undef ShellClass_TableText
#define ShellClass TableText
#endif

/************************************************************/
#ifdef ShellClass_ScEditShell
#undef ShellClass
#undef ShellClass_ScEditShell
#define ShellClass ScEditShell
static SfxFormalArgument aScEditShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxUInt32Item_Impl, "Format", SID_PASTE_SPECIAL },
     { (const SfxType*) &aSfxUInt32Item_Impl, "SelectedFormat", SID_CLIPBOARD_FORMAT_ITEMS },
     { (const SfxType*) &aSfxStringItem_Impl, "WordReplace", SID_THES },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxColorItem_Impl, "Color", SID_ATTR_CHAR_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "Page", FN_PARAM_1 },
     { (const SfxType*) &aSvxHyperlinkItem_Impl, "Hyperlink", SID_HYPERLINK_SETLINK },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbols", SID_CHARMAP },
     { (const SfxType*) &aSfxStringItem_Impl, "FontName", SID_ATTR_SPECIALCHAR }
    };

SFX_EXEC_STUB(ScEditShell,Execute)
SFX_STATE_STUB(ScEditShell,GetClipState)
SFX_STATE_STUB(ScEditShell,GetState)
SFX_EXEC_STUB(ScEditShell,ExecuteUndo)
SFX_STATE_STUB(ScEditShell,GetUndoState)
SFX_EXEC_STUB(ScEditShell,ExecuteAttr)
SFX_STATE_STUB(ScEditShell,GetAttrState)
SFX_EXEC_STUB(ScEditShell,ExecuteTrans)

static SfxSlot aScEditShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5311
     SFX_NEW_SLOT_ARG( ScEditShell,SID_PASTE_SPECIAL,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetClipState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteSpecial" ),
     // Slot Nr. 1 : 5312
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CLIPBOARD_FORMAT_ITEMS,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxClipboardFormatItem,
                       1/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClipboardFormatItems" ),
     // Slot Nr. 2 : 5314
     SFX_NEW_SLOT_ARG( ScEditShell,SID_PASTE_UNFORMATTED,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetClipState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteUnformatted" ),
     // Slot Nr. 3 : 5405
     SFX_NEW_SLOT_ARG( ScEditShell,SID_EMOJI_CONTROL,SfxGroupId::Special,
                       &aScEditShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"EmojiControl" ),
     // Slot Nr. 4 : 5406
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CHARMAP_CONTROL,SfxGroupId::Special,
                       &aScEditShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"CharmapControl" ),
     // Slot Nr. 5 : 5698
     SFX_NEW_SLOT_ARG( ScEditShell,SID_THES,SfxGroupId::Text,
                       &aScEditShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusFromContext" ),
     // Slot Nr. 6 : 5700
     SFX_NEW_SLOT_ARG( ScEditShell,SID_REDO,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteUndo),SFX_STUB_PTR(ScEditShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       3/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 7 : 5701
     SFX_NEW_SLOT_ARG( ScEditShell,SID_UNDO,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteUndo),SFX_STUB_PTR(ScEditShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       5/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 8 : 5702
     SFX_NEW_SLOT_ARG( ScEditShell,SID_REPEAT,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteUndo),SFX_STUB_PTR(ScEditShell,GetUndoState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       7/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Repeat" ),
     // Slot Nr. 9 : 5710
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CUT,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       7/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 10 : 5711
     SFX_NEW_SLOT_ARG( ScEditShell,SID_COPY,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       7/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 11 : 5712
     SFX_NEW_SLOT_ARG( ScEditShell,SID_PASTE,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       7/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 12 : 5713
     SFX_NEW_SLOT_ARG( ScEditShell,SID_DELETE,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       9/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 13 : 5723
     SFX_NEW_SLOT_ARG( ScEditShell,SID_SELECTALL,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       9/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 14 : 5792
     SFX_NEW_SLOT_ARG( ScEditShell,SID_UNICODE_NOTATION_TOGGLE,SfxGroupId::Options,
                       &aScEditShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       9/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UnicodeNotationToggle" ),
     // Slot Nr. 15 : 10007
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_FONT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"CharFontName" ),
     // Slot Nr. 16 : 10008
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_POSTURE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPostureItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"Italic" ),
     // Slot Nr. 17 : 10009
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_WEIGHT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxWeightItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"Bold" ),
     // Slot Nr. 18 : 10010
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_SHADOWED,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxShadowedItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"Shadowed" ),
     // Slot Nr. 19 : 10012
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_CONTOUR,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxContourItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"OutlineFont" ),
     // Slot Nr. 20 : 10013
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_STRIKEOUT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCrossedOutItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"Strikeout" ),
     // Slot Nr. 21 : 10014
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_UNDERLINE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxUnderlineItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"Underline" ),
     // Slot Nr. 22 : 10015
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_FONTHEIGHT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontHeightItem,
                       9/*Offset*/, 0, SfxSlotMode::NONE,"FontHeight" ),
     // Slot Nr. 23 : 10017
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_COLOR,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       9/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Color" ),
     // Slot Nr. 24 : 10018
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_KERNING,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxKerningItem,
                       11/*Offset*/, 0, SfxSlotMode::NONE,"Spacing" ),
     // Slot Nr. 25 : 10221
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_INSERT,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       11/*Offset*/, 0, SfxSlotMode::NONE,"InsertMode" ),
     // Slot Nr. 26 : 10294
     SFX_NEW_SLOT_ARG( ScEditShell,SID_SET_SUPER_SCRIPT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       11/*Offset*/, 0, SfxSlotMode::NONE,"SuperScript" ),
     // Slot Nr. 27 : 10295
     SFX_NEW_SLOT_ARG( ScEditShell,SID_SET_SUB_SCRIPT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       11/*Offset*/, 0, SfxSlotMode::NONE,"SubScript" ),
     // Slot Nr. 28 : 10296
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CHAR_DLG,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       11/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontDialog" ),
     // Slot Nr. 29 : 10361
     SFX_NEW_SLOT_ARG( ScEditShell,SID_HYPERLINK_GETLINK,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHyperlinkItem,
                       12/*Offset*/, 0, SfxSlotMode::NONE,"Hyperlink" ),
     // Slot Nr. 30 : 10362
     SFX_NEW_SLOT_ARG( ScEditShell,SID_HYPERLINK_SETLINK,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       12/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetHyperlink" ),
     // Slot Nr. 31 : 10459
     SFX_NEW_SLOT_ARG( ScEditShell,SID_REMOVE_HYPERLINK,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       13/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RemoveHyperlink" ),
     // Slot Nr. 32 : 10503
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CHARMAP,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       13/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSymbol" ),
     // Slot Nr. 33 : 10912
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_UPPER,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToUpper" ),
     // Slot Nr. 34 : 10913
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_LOWER,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToLower" ),
     // Slot Nr. 35 : 10914
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_HALFWIDTH,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHalfWidth" ),
     // Slot Nr. 36 : 10915
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_FULLWIDTH,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToFullWidth" ),
     // Slot Nr. 37 : 10916
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_HIRAGANA,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHiragana" ),
     // Slot Nr. 38 : 10917
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_KATAKANA,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToKatakana" ),
     // Slot Nr. 39 : 10955
     SFX_NEW_SLOT_ARG( ScEditShell,SID_OPEN_HYPERLINK,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenHyperlinkOnCursor" ),
     // Slot Nr. 40 : 10978
     SFX_NEW_SLOT_ARG( ScEditShell,SID_EDIT_HYPERLINK,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditHyperlink" ),
     // Slot Nr. 41 : 11042
     SFX_NEW_SLOT_ARG( ScEditShell,SID_GROW_FONT_SIZE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Grow" ),
     // Slot Nr. 42 : 11043
     SFX_NEW_SLOT_ARG( ScEditShell,SID_SHRINK_FONT_SIZE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Shrink" ),
     // Slot Nr. 43 : 11056
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_RLM,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRLM" ),
     // Slot Nr. 44 : 11057
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_LRM,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertLRM" ),
     // Slot Nr. 45 : 11058
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_ZWSP,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertZWSP" ),
     // Slot Nr. 46 : 11059
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_ZWNBSP,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertZWNBSP" ),
     // Slot Nr. 47 : 11102
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_SENTENCE_CASE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToSentenceCase" ),
     // Slot Nr. 48 : 11103
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_TITLE_CASE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToTitleCase" ),
     // Slot Nr. 49 : 11104
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TRANSLITERATE_TOGGLE_CASE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteTrans),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToToggleCase" ),
     // Slot Nr. 50 : 11133
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CHAR_DLG_EFFECT,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontEffectsDialog" ),
     // Slot Nr. 51 : 11193
     SFX_NEW_SLOT_ARG( ScEditShell,SID_COPY_HYPERLINK_LOCATION,SfxGroupId::Edit,
                       &aScEditShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0, SfxSlotMode::NONE,"CopyHyperlinkLocation" ),
     // Slot Nr. 52 : 11195
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ULINE_VAL_NONE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       15/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineNone" ),
     // Slot Nr. 53 : 11196
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ULINE_VAL_SINGLE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       15/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineSingle" ),
     // Slot Nr. 54 : 11197
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ULINE_VAL_DOUBLE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       15/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDouble" ),
     // Slot Nr. 55 : 11198
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ULINE_VAL_DOTTED,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       15/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDotted" ),
     // Slot Nr. 56 : 11568
     SFX_NEW_SLOT_ARG( ScEditShell,SID_ATTR_CHAR_OVERLINE,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,ExecuteAttr),SFX_STUB_PTR(ScEditShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxOverlineItem,
                       15/*Offset*/, 0, SfxSlotMode::NONE,"Overline" ),
     // Slot Nr. 57 : 20343
     SFX_NEW_SLOT_ARG( ScEditShell,FN_INSERT_SOFT_HYPHEN,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSoftHyphen" ),
     // Slot Nr. 58 : 20344
     SFX_NEW_SLOT_ARG( ScEditShell,FN_INSERT_HARD_SPACE,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertNonBreakingSpace" ),
     // Slot Nr. 59 : 20345
     SFX_NEW_SLOT_ARG( ScEditShell,FN_INSERT_NNBSP,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertNarrowNobreakSpace" ),
     // Slot Nr. 60 : 20385
     SFX_NEW_SLOT_ARG( ScEditShell,FN_INSERT_HARDHYPHEN,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertHardHyphen" ),
     // Slot Nr. 61 : 26067
     SFX_NEW_SLOT_ARG( ScEditShell,SID_CELL_FORMAT_RESET,SfxGroupId::Format,
                       &aScEditShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ResetAttributes" ),
     // Slot Nr. 62 : 26272
     SFX_NEW_SLOT_ARG( ScEditShell,FID_INSERT_NAME,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertName" ),
     // Slot Nr. 63 : 26609
     SFX_NEW_SLOT_ARG( ScEditShell,SID_TOGGLE_REL,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleRelative" ),
     // Slot Nr. 64 : 26668
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_FIELD_SHEET,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFieldSheet" ),
     // Slot Nr. 65 : 26669
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_FIELD_TITLE,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFieldDocTitle" ),
     // Slot Nr. 66 : 26670
     SFX_NEW_SLOT_ARG( ScEditShell,SID_INSERT_FIELD_DATE_VAR,SfxGroupId::Insert,
                       &aScEditShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScEditShell,Execute),SFX_STUB_PTR(ScEditShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFieldDateVariable" )
    };
#endif

/************************************************************/
#ifdef ShellClass_Pivot
#undef ShellClass
#undef ShellClass_Pivot
#define ShellClass Pivot
#endif

/************************************************************/
#ifdef ShellClass_ScPivotShell
#undef ShellClass
#undef ShellClass_ScPivotShell
#define ShellClass ScPivotShell
static SfxFormalArgument aScPivotShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };

SFX_EXEC_STUB(ScPivotShell,Execute)
SFX_STATE_STUB(ScPivotShell,GetState)

static SfxSlot aScPivotShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 26091
     SFX_NEW_SLOT_ARG( ScPivotShell,SID_DP_FILTER,SfxGroupId::Data,
                       &aScPivotShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScPivotShell,Execute),SFX_STUB_PTR(ScPivotShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataPilotFilter" ),
     // Slot Nr. 1 : 26321
     SFX_NEW_SLOT_ARG( ScPivotShell,SID_PIVOT_RECALC,SfxGroupId::Data,
                       &aScPivotShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScPivotShell,Execute),SFX_STUB_PTR(ScPivotShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RecalcPivotTable" ),
     // Slot Nr. 2 : 26322
     SFX_NEW_SLOT_ARG( ScPivotShell,SID_PIVOT_KILL,SfxGroupId::Data,
                       &aScPivotShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScPivotShell,Execute),SFX_STUB_PTR(ScPivotShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeletePivotTable" )
    };
#endif

/************************************************************/
#ifdef ShellClass_StarCalc
#undef ShellClass
#undef ShellClass_StarCalc
#define ShellClass StarCalc
#endif

/************************************************************/
#ifdef ShellClass_ScModule
#undef ShellClass
#undef ShellClass_ScModule
#define ShellClass ScModule
static SfxFormalArgument aScModuleArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };

SFX_EXEC_STUB(ScModule,Execute)
SFX_STATE_STUB(ScModule,GetState)
SFX_STATE_STUB(ScModule,HideDisabledSlots)

static SfxSlot aScModuleSlots_Impl[] =
    {
     // Slot Nr. 0 : 10423
     SFX_NEW_SLOT_ARG( ScModule,SID_PSZ_FUNCTION,SfxGroupId::Intern,
                       &aScModuleSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"StatusBarFunc" ),
     // Slot Nr. 1 : 10889
     SFX_NEW_SLOT_ARG( ScModule,SID_ATTR_CHAR_CJK_LANGUAGE,SfxGroupId::Edit,
                       &aScModuleSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLanguageItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"DocumentLanguageCJK" ),
     // Slot Nr. 2 : 10894
     SFX_NEW_SLOT_ARG( ScModule,SID_ATTR_CHAR_CTL_LANGUAGE,SfxGroupId::Edit,
                       &aScModuleSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLanguageItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"DocumentLanguageCTL" ),
     // Slot Nr. 3 : 10914
     SFX_NEW_SLOT_ARG( ScModule,SID_TRANSLITERATE_HALFWIDTH,SfxGroupId::Format,
                       &aScModuleSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHalfWidth" ),
     // Slot Nr. 4 : 10915
     SFX_NEW_SLOT_ARG( ScModule,SID_TRANSLITERATE_FULLWIDTH,SfxGroupId::Format,
                       &aScModuleSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToFullWidth" ),
     // Slot Nr. 5 : 10916
     SFX_NEW_SLOT_ARG( ScModule,SID_TRANSLITERATE_HIRAGANA,SfxGroupId::Format,
                       &aScModuleSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHiragana" ),
     // Slot Nr. 6 : 10917
     SFX_NEW_SLOT_ARG( ScModule,SID_TRANSLITERATE_KATAKANA,SfxGroupId::Format,
                       &aScModuleSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToKatakana" ),
     // Slot Nr. 7 : 10958
     SFX_NEW_SLOT_ARG( ScModule,SID_OPEN_XML_FILTERSETTINGS,SfxGroupId::Options,
                       &aScModuleSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenXMLFilterSettings" ),
     // Slot Nr. 8 : 10959
     SFX_NEW_SLOT_ARG( ScModule,SID_HANGUL_HANJA_CONVERSION,SfxGroupId::Options,
                       &aScModuleSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HangulHanjaConversion" ),
     // Slot Nr. 9 : 11016
     SFX_NEW_SLOT_ARG( ScModule,SID_CHINESE_CONVERSION,SfxGroupId::Options,
                       &aScModuleSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChineseConversion" ),
     // Slot Nr. 10 : 11056
     SFX_NEW_SLOT_ARG( ScModule,SID_INSERT_RLM,SfxGroupId::Insert,
                       &aScModuleSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRLM" ),
     // Slot Nr. 11 : 11057
     SFX_NEW_SLOT_ARG( ScModule,SID_INSERT_LRM,SfxGroupId::Insert,
                       &aScModuleSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScModule,HideDisabledSlots),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertLRM" ),
     // Slot Nr. 12 : 12007
     SFX_NEW_SLOT_ARG( ScModule,SID_ATTR_LANGUAGE,SfxGroupId::Edit,
                       &aScModuleSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLanguageItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"DocumentLanguage" ),
     // Slot Nr. 13 : 12008
     SFX_NEW_SLOT_ARG( ScModule,SID_ATTR_METRIC,SfxGroupId::NONE,
                       &aScModuleSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"MetricUnit" ),
     // Slot Nr. 14 : 12021
     SFX_NEW_SLOT_ARG( ScModule,SID_AUTOSPELL_CHECK,SfxGroupId::Options,
                       &aScModuleSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"SpellOnline" ),
     // Slot Nr. 15 : 26082
     SFX_NEW_SLOT_ARG( ScModule,SID_CHOOSE_DESIGN,SfxGroupId::Format,
                       &aScModuleSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChooseDesign" ),
     // Slot Nr. 16 : 26083
     SFX_NEW_SLOT_ARG( ScModule,SID_EURO_CONVERTER,SfxGroupId::Format,
                       &aScModuleSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EuroConverter" ),
     // Slot Nr. 17 : 26326
     SFX_NEW_SLOT_ARG( ScModule,FID_AUTOCOMPLETE,SfxGroupId::Options,
                       &aScModuleSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"AutoComplete" ),
     // Slot Nr. 18 : 26482
     SFX_NEW_SLOT_ARG( ScModule,SID_DETECTIVE_AUTO,SfxGroupId::Options,
                       &aScModuleSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR(ScModule,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"AutoRefreshArrows" ),
     // Slot Nr. 19 : 26645
     SFX_NEW_SLOT_ARG( ScModule,FID_FOCUS_POSWND,SfxGroupId::Application,
                       &aScModuleSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScModule,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FocusCellAddress" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TableDrawText
#undef ShellClass
#undef ShellClass_TableDrawText
#define ShellClass TableDrawText
#endif

/************************************************************/
#ifdef ShellClass_ScDrawTextObjectBar
#undef ShellClass
#undef ShellClass_ScDrawTextObjectBar
#define ShellClass ScDrawTextObjectBar
static SfxFormalArgument aScDrawTextObjectBarArgs_Impl[] =
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
     { (const SfxType*) &aSfxStringItem_Impl, "WordReplace", SID_THES },
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_HIDE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_SHOW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxColorItem_Impl, "Color", SID_ATTR_CHAR_COLOR },
     { (const SfxType*) &aSvxLRSpaceItem_Impl, "LRSpace", SID_ATTR_PARA_LRSPACE },
     { (const SfxType*) &aSfxStringItem_Impl, "Page", FN_PARAM_1 },
     { (const SfxType*) &aSvxHyperlinkItem_Impl, "Hyperlink", SID_HYPERLINK_SETLINK },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbols", SID_CHARMAP },
     { (const SfxType*) &aSfxStringItem_Impl, "FontName", SID_ATTR_SPECIALCHAR },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxBackgroundColorItem_Impl, "CharBackColor", SID_ATTR_CHAR_BACK_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_DEFINE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbol", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Options", FN_PARAM_2 }
    };

SFX_EXEC_STUB(ScDrawTextObjectBar,Execute)
SFX_STATE_STUB(ScDrawTextObjectBar,GetClipState)
SFX_STATE_STUB(ScDrawTextObjectBar,GetState)
SFX_STATE_STUB(ScDrawTextObjectBar,StateDisableItems)
SFX_EXEC_STUB(ScDrawTextObjectBar,ExecuteAttr)
SFX_STATE_STUB(ScDrawTextObjectBar,GetAttrState)
SFX_EXEC_STUB(ScDrawTextObjectBar,ExecuteExtra)
SFX_EXEC_STUB(ScDrawTextObjectBar,ExecFormText)
SFX_STATE_STUB(ScDrawTextObjectBar,GetFormTextState)
SFX_EXEC_STUB(ScDrawTextObjectBar,ExecuteTrans)
SFX_STATE_STUB(ScDrawTextObjectBar,GetStatePropPanelAttr)
SFX_EXEC_STUB(ScDrawTextObjectBar,ExecuteToggle)

static SfxSlot aScDrawTextObjectBarSlots_Impl[] =
    {
     // Slot Nr. 0 : 5311
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_PASTE_SPECIAL,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetClipState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteSpecial" ),
     // Slot Nr. 1 : 5312
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CLIPBOARD_FORMAT_ITEMS,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxClipboardFormatItem,
                       1/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClipboardFormatItems" ),
     // Slot Nr. 2 : 5314
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_PASTE_UNFORMATTED,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetClipState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteUnformatted" ),
     // Slot Nr. 3 : 5405
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_EMOJI_CONTROL,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"EmojiControl" ),
     // Slot Nr. 4 : 5406
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CHARMAP_CONTROL,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"CharmapControl" ),
     // Slot Nr. 5 : 5542
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_FAMILY2,SfxGroupId::Document,
                       &aScDrawTextObjectBarSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"ParaStyle" ),
     // Slot Nr. 6 : 5544
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_FAMILY4,SfxGroupId::Document,
                       &aScDrawTextObjectBarSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"PageStyle" ),
     // Slot Nr. 7 : 5549
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_NEW,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewStyle" ),
     // Slot Nr. 8 : 5550
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_EDIT,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditStyle" ),
     // Slot Nr. 9 : 5551
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_DELETE,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       6/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteStyle" ),
     // Slot Nr. 10 : 5552
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_APPLY,SfxGroupId::Document,
                       &aScDrawTextObjectBarSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       8/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleApply" ),
     // Slot Nr. 11 : 5554
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_WATERCAN,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       12/*Offset*/, 0, SfxSlotMode::NONE,"StyleWatercanMode" ),
     // Slot Nr. 12 : 5555
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_NEW_BY_EXAMPLE,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       12/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleNewByExample" ),
     // Slot Nr. 13 : 5556
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_UPDATE_BY_EXAMPLE,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleUpdateByExample" ),
     // Slot Nr. 14 : 5698
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_THES,SfxGroupId::Text,
                       &aScDrawTextObjectBarSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusFromContext" ),
     // Slot Nr. 15 : 5710
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CUT,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 16 : 5711
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_COPY,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 17 : 5712
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_PASTE,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 18 : 5723
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_SELECTALL,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 19 : 6603
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_HIDE,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideStyle" ),
     // Slot Nr. 20 : 6604
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_STYLE_SHOW,SfxGroupId::Template,
                       &aScDrawTextObjectBarSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowStyle" ),
     // Slot Nr. 21 : 10007
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_FONT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"CharFontName" ),
     // Slot Nr. 22 : 10008
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_POSTURE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPostureItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Italic" ),
     // Slot Nr. 23 : 10009
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_WEIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxWeightItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Bold" ),
     // Slot Nr. 24 : 10010
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_SHADOWED,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxShadowedItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Shadowed" ),
     // Slot Nr. 25 : 10012
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_CONTOUR,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxContourItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"OutlineFont" ),
     // Slot Nr. 26 : 10013
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_STRIKEOUT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCrossedOutItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Strikeout" ),
     // Slot Nr. 27 : 10014
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_UNDERLINE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxUnderlineItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Underline" ),
     // Slot Nr. 28 : 10015
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_FONTHEIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontHeightItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"FontHeight" ),
     // Slot Nr. 29 : 10017
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_COLOR,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       23/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Color" ),
     // Slot Nr. 30 : 10018
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_KERNING,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxKerningItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"Spacing" ),
     // Slot Nr. 31 : 10028
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_ADJUST_LEFT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"LeftPara" ),
     // Slot Nr. 32 : 10029
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_ADJUST_RIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"RightPara" ),
     // Slot Nr. 33 : 10030
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_ADJUST_CENTER,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"CenterPara" ),
     // Slot Nr. 34 : 10031
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_ADJUST_BLOCK,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"JustifyPara" ),
     // Slot Nr. 35 : 10033
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_LINESPACE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineSpacingItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"LineSpacing" ),
     // Slot Nr. 36 : 10034
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_LINESPACE_10,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"SpacePara1" ),
     // Slot Nr. 37 : 10035
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_LINESPACE_15,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"SpacePara15" ),
     // Slot Nr. 38 : 10036
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_LINESPACE_20,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"SpacePara2" ),
     // Slot Nr. 39 : 10042
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_ULSPACE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxULSpaceItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"ULSpacing" ),
     // Slot Nr. 40 : 10043
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_LRSPACE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLRSpaceItem,
                       25/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LeftRightParaMargin" ),
     // Slot Nr. 41 : 10245
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_THESAURUS,SfxGroupId::Options,
                       &aScDrawTextObjectBarSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       26/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusDialog" ),
     // Slot Nr. 42 : 10256
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FONTWORK,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteExtra),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWork" ),
     // Slot Nr. 43 : 10257
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_STYLE,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextStyleItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextStyle" ),
     // Slot Nr. 44 : 10258
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_ADJUST,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextAdjustItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextAdjust" ),
     // Slot Nr. 45 : 10259
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_DISTANCE,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextDistanceItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextDistance" ),
     // Slot Nr. 46 : 10260
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_START,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextStartItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextStart" ),
     // Slot Nr. 47 : 10261
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_MIRROR,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextMirrorItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextMirror" ),
     // Slot Nr. 48 : 10262
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_OUTLINE,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextOutlineItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextOutline" ),
     // Slot Nr. 49 : 10263
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_SHADOW,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadow" ),
     // Slot Nr. 50 : 10264
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_SHDWCOLOR,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowColorItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowColor" ),
     // Slot Nr. 51 : 10265
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_SHDWXVAL,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowXValItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowXVal" ),
     // Slot Nr. 52 : 10266
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_SHDWYVAL,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextShadowYValItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FontWorkTextShadowYVal" ),
     // Slot Nr. 53 : 10268
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FORMTEXT_HIDEFORM,SfxGroupId::Special,
                       &aScDrawTextObjectBarSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecFormText),SFX_STUB_PTR(ScDrawTextObjectBar,GetFormTextState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       XFormTextHideFormItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"FormatFontWorkClose" ),
     // Slot Nr. 54 : 10294
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_SET_SUPER_SCRIPT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"SuperScript" ),
     // Slot Nr. 55 : 10295
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_SET_SUB_SCRIPT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       26/*Offset*/, 0, SfxSlotMode::NONE,"SubScript" ),
     // Slot Nr. 56 : 10296
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CHAR_DLG,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       26/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontDialog" ),
     // Slot Nr. 57 : 10297
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_PARA_DLG,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ParagraphDialog" ),
     // Slot Nr. 58 : 10361
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_HYPERLINK_GETLINK,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHyperlinkItem,
                       27/*Offset*/, 0, SfxSlotMode::NONE,"Hyperlink" ),
     // Slot Nr. 59 : 10362
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_HYPERLINK_SETLINK,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetHyperlink" ),
     // Slot Nr. 60 : 10459
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_REMOVE_HYPERLINK,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RemoveHyperlink" ),
     // Slot Nr. 61 : 10503
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CHARMAP,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSymbol" ),
     // Slot Nr. 62 : 10907
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TEXTDIRECTION_LEFT_TO_RIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"TextdirectionLeftToRight" ),
     // Slot Nr. 63 : 10908
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TEXTDIRECTION_TOP_TO_BOTTOM,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"TextdirectionTopToBottom" ),
     // Slot Nr. 64 : 10912
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_UPPER,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToUpper" ),
     // Slot Nr. 65 : 10913
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_LOWER,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToLower" ),
     // Slot Nr. 66 : 10914
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_HALFWIDTH,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHalfWidth" ),
     // Slot Nr. 67 : 10915
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_FULLWIDTH,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToFullWidth" ),
     // Slot Nr. 68 : 10916
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_HIRAGANA,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHiragana" ),
     // Slot Nr. 69 : 10917
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_KATAKANA,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToKatakana" ),
     // Slot Nr. 70 : 10950
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_LEFT_TO_RIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteExtra),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"ParaLeftToRight" ),
     // Slot Nr. 71 : 10951
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_PARA_RIGHT_TO_LEFT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteExtra),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"ParaRightToLeft" ),
     // Slot Nr. 72 : 10955
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPEN_HYPERLINK,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"OpenHyperlinkOnCursor" ),
     // Slot Nr. 73 : 10956
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CTLFONT_STATE,SfxGroupId::Application,
                       &aScDrawTextObjectBarSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"CTLFontState" ),
     // Slot Nr. 74 : 10957
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_VERTICALTEXT_STATE,SfxGroupId::Application,
                       &aScDrawTextObjectBarSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"VerticalTextState" ),
     // Slot Nr. 75 : 10978
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_EDIT_HYPERLINK,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditHyperlink" ),
     // Slot Nr. 76 : 11002
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGN_ANY_LEFT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignLeft" ),
     // Slot Nr. 77 : 11003
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGN_ANY_HCENTER,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignHorizontalCenter" ),
     // Slot Nr. 78 : 11004
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGN_ANY_RIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignRight" ),
     // Slot Nr. 79 : 11005
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGN_ANY_JUSTIFIED,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignJustified" ),
     // Slot Nr. 80 : 11042
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_GROW_FONT_SIZE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Grow" ),
     // Slot Nr. 81 : 11043
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_SHRINK_FONT_SIZE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Shrink" ),
     // Slot Nr. 82 : 11073
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TABLE_VERT_BOTTOM,SfxGroupId::Table,
                       &aScDrawTextObjectBarSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetStatePropPanelAttr),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"CellVertBottom" ),
     // Slot Nr. 83 : 11074
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TABLE_VERT_CENTER,SfxGroupId::Table,
                       &aScDrawTextObjectBarSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetStatePropPanelAttr),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"CellVertCenter" ),
     // Slot Nr. 84 : 11075
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TABLE_VERT_NONE,SfxGroupId::Table,
                       &aScDrawTextObjectBarSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetStatePropPanelAttr),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"CellVertTop" ),
     // Slot Nr. 85 : 11102
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_SENTENCE_CASE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToSentenceCase" ),
     // Slot Nr. 86 : 11103
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_TITLE_CASE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToTitleCase" ),
     // Slot Nr. 87 : 11104
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TRANSLITERATE_TOGGLE_CASE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteTrans),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToToggleCase" ),
     // Slot Nr. 88 : 11133
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_CHAR_DLG_EFFECT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontEffectsDialog" ),
     // Slot Nr. 89 : 11145
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_PARASPACE_INCREASE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ParaspaceIncrease" ),
     // Slot Nr. 90 : 11146
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_PARASPACE_DECREASE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ParaspaceDecrease" ),
     // Slot Nr. 91 : 11153
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_BACK_COLOR,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBackgroundColorItem,
                       30/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CharBackColor" ),
     // Slot Nr. 92 : 11193
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_COPY_HYPERLINK_LOCATION,SfxGroupId::Edit,
                       &aScDrawTextObjectBarSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"CopyHyperlinkLocation" ),
     // Slot Nr. 93 : 11195
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ULINE_VAL_NONE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteToggle),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineNone" ),
     // Slot Nr. 94 : 11196
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ULINE_VAL_SINGLE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteToggle),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineSingle" ),
     // Slot Nr. 95 : 11197
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ULINE_VAL_DOUBLE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteToggle),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDouble" ),
     // Slot Nr. 96 : 11198
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ULINE_VAL_DOTTED,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteToggle),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDotted" ),
     // Slot Nr. 97 : 11568
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ATTR_CHAR_OVERLINE,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[113] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxOverlineItem,
                       32/*Offset*/, 0, SfxSlotMode::NONE,"Overline" ),
     // Slot Nr. 98 : 26071
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_DRAW_CHART,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DrawChart" ),
     // Slot Nr. 99 : 26087
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ENABLE_HYPHENATION,SfxGroupId::Options,
                       &aScDrawTextObjectBarSlots_Impl[108] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,Execute),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Hyphenate" ),
     // Slot Nr. 100 : 26150
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_CONSOLIDATE,SfxGroupId::Data,
                       &aScDrawTextObjectBarSlots_Impl[101] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataConsolidate" ),
     // Slot Nr. 101 : 26151
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_PIVOTTABLE,SfxGroupId::Data,
                       &aScDrawTextObjectBarSlots_Impl[102] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataDataPilotRun" ),
     // Slot Nr. 102 : 26152
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_FUNCTION,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[103] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FunctionDialog" ),
     // Slot Nr. 103 : 26153
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_SOLVE,SfxGroupId::Options,
                       &aScDrawTextObjectBarSlots_Impl[104] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoalSeekDialog" ),
     // Slot Nr. 104 : 26154
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_TABOP,SfxGroupId::Data,
                       &aScDrawTextObjectBarSlots_Impl[105] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableOperationDialog" ),
     // Slot Nr. 105 : 26160
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_OPTSOLVER,SfxGroupId::Options,
                       &aScDrawTextObjectBarSlots_Impl[106] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SolverDialog" ),
     // Slot Nr. 106 : 26271
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,FID_DEFINE_NAME,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[107] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineName" ),
     // Slot Nr. 107 : 26276
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,FID_ADD_NAME,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[110] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AddName" ),
     // Slot Nr. 108 : 26303
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_TEXT_STANDARD,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[109] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StandardTextAttributes" ),
     // Slot Nr. 109 : 26304
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_DRAWTEXT_ATTR_DLG,SfxGroupId::Drawing,
                       &aScDrawTextObjectBarSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TextAttributes" ),
     // Slot Nr. 110 : 26327
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_DEFINE_DBNAME,SfxGroupId::Data,
                       &aScDrawTextObjectBarSlots_Impl[111] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineDBName" ),
     // Slot Nr. 111 : 26330
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_FILTER,SfxGroupId::Data,
                       &aScDrawTextObjectBarSlots_Impl[112] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterStandardFilter" ),
     // Slot Nr. 112 : 26331
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_SPECIAL_FILTER,SfxGroupId::Data,
                       &aScDrawTextObjectBarSlots_Impl[117] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterSpecialFilter" ),
     // Slot Nr. 113 : 26382
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGNLEFT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[114] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignLeft" ),
     // Slot Nr. 114 : 26383
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGNRIGHT,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[115] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignRight" ),
     // Slot Nr. 115 : 26384
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGNCENTERHOR,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[116] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignHorizontalCenter" ),
     // Slot Nr. 116 : 26385
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_ALIGNBLOCK,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScDrawTextObjectBar,ExecuteAttr),SFX_STUB_PTR(ScDrawTextObjectBar,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignBlock" ),
     // Slot Nr. 117 : 26605
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_OPENDLG_EDIT_PRINTAREA,SfxGroupId::Format,
                       &aScDrawTextObjectBarSlots_Impl[118] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditPrintArea" ),
     // Slot Nr. 118 : 26629
     SFX_NEW_SLOT_ARG( ScDrawTextObjectBar,SID_DEFINE_COLROWNAMERANGES,SfxGroupId::Insert,
                       &aScDrawTextObjectBarSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScDrawTextObjectBar,StateDisableItems),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       35/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineLabelRange" )
    };
#endif

/************************************************************/
#ifdef ShellClass_TablePrintPreview
#undef ShellClass
#undef ShellClass_TablePrintPreview
#define ShellClass TablePrintPreview
#endif

/************************************************************/
#ifdef ShellClass_ScPreviewShell
#undef ShellClass
#undef ShellClass_ScPreviewShell
#define ShellClass ScPreviewShell
static SfxFormalArgument aScPreviewShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxStringItem_Impl, "Recipient", SID_MAIL_RECIPIENT },
     { (const SfxType*) &aSfxStringItem_Impl, "Subject", SID_MAIL_SUBJECT },
     { (const SfxType*) &aSfxStringItem_Impl, "MailText", SID_MAIL_TEXT },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Priority", SID_MAIL_PRIORITY },
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
     { (const SfxType*) &aSfxUInt16Item_Impl, "Redo", SID_REDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Undo", SID_UNDO },
     { (const SfxType*) &aSfxBoolItem_Impl, "Repair", SID_REPAIRPACKAGE },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", SID_CURSORDOWN },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", SID_CURSORUP },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", SID_CURSORLEFT },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", SID_CURSORRIGHT },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", SID_CURSORPAGEDOWN },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", SID_CURSORPAGEUP },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 }
    };

SFX_STATE_STUB(ScPreviewShell,GetState)
SFX_EXEC_STUB(ScPreviewShell,Execute)

static SfxSlot aScPreviewShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5300
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_QUITAPP,SfxGroupId::Application,
                       &aScPreviewShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Quit" ),
     // Slot Nr. 1 : 5325
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PRINTPREVIEW,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"PrintPreview" ),
     // Slot Nr. 2 : 5331
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_MAIL_SENDDOC,SfxGroupId::Document,
                       &aScPreviewShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       0/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SendMail" ),
     // Slot Nr. 3 : 5502
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_SAVEASDOC,SfxGroupId::Document,
                       &aScPreviewShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       4/*Offset*/, 12/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveAs" ),
     // Slot Nr. 4 : 5505
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_SAVEDOC,SfxGroupId::Document,
                       &aScPreviewShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       16/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Save" ),
     // Slot Nr. 5 : 5700
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_REDO,SfxGroupId::Edit,
                       &aScPreviewShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       20/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Redo" ),
     // Slot Nr. 6 : 5701
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_UNDO,SfxGroupId::Edit,
                       &aScPreviewShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       22/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Undo" ),
     // Slot Nr. 7 : 5702
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_REPEAT,SfxGroupId::Edit,
                       &aScPreviewShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       24/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Repeat" ),
     // Slot Nr. 8 : 5731
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORDOWN,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       24/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDown" ),
     // Slot Nr. 9 : 5732
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORUP,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUp" ),
     // Slot Nr. 10 : 5733
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORLEFT,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       26/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeft" ),
     // Slot Nr. 11 : 5734
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORRIGHT,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       27/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRight" ),
     // Slot Nr. 12 : 5735
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORPAGEDOWN,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDownBlock" ),
     // Slot Nr. 13 : 5736
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORPAGEUP,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUpBlock" ),
     // Slot Nr. 14 : 5741
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORENDOFFILE,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToEndOfData" ),
     // Slot Nr. 15 : 5742
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORTOPOFFILE,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToStart" ),
     // Slot Nr. 16 : 5745
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSORHOME,SfxGroupId::Navigator,
                       &aScPreviewShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToStartOfRow" ),
     // Slot Nr. 17 : 5746
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CURSOREND,SfxGroupId::Navigator,
                       &aScPreviewShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToEndOfRow" ),
     // Slot Nr. 18 : 6660
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_VIEW_DATA_SOURCE_BROWSER,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       31/*Offset*/, 0, SfxSlotMode::NONE,"ViewDataSourceBrowser" ),
     // Slot Nr. 19 : 10000
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_ATTR_ZOOM,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomItem,
                       31/*Offset*/, 0, SfxSlotMode::NONE,"Zoom" ),
     // Slot Nr. 20 : 11065
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_ATTR_ZOOMSLIDER,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomSliderItem,
                       31/*Offset*/, 0, SfxSlotMode::NONE,"ZoomSlider" ),
     // Slot Nr. 21 : 26012
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_REPAINT,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Repaint" ),
     // Slot Nr. 22 : 26114
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_STATUS_DOCPOS,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       31/*Offset*/, 0, SfxSlotMode::NONE,"StatusDocPos" ),
     // Slot Nr. 23 : 26115
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_STATUS_PAGESTYLE,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       31/*Offset*/, 0, SfxSlotMode::NONE,"StatusPageStyle" ),
     // Slot Nr. 24 : 26235
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_HFEDIT,SfxGroupId::Edit,
                       &aScPreviewShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditHeaderAndFooter" ),
     // Slot Nr. 25 : 26244
     SFX_NEW_SLOT_ARG( ScPreviewShell,FID_SCALE,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Scale" ),
     // Slot Nr. 26 : 26302
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_FORMATPAGE,SfxGroupId::Format,
                       &aScPreviewShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PageFormatDialog" ),
     // Slot Nr. 27 : 26507
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_NEXT,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NextPage" ),
     // Slot Nr. 28 : 26508
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_PREVIOUS,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PreviousPage" ),
     // Slot Nr. 29 : 26509
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_FIRST,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FirstPage" ),
     // Slot Nr. 30 : 26510
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_LAST,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"LastPage" ),
     // Slot Nr. 31 : 26512
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_ZOOMIN,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomIn" ),
     // Slot Nr. 32 : 26513
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_ZOOMOUT,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZoomOut" ),
     // Slot Nr. 33 : 26514
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_CLOSE,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClosePreview" ),
     // Slot Nr. 34 : 26515
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_MARGIN,SfxGroupId::View,
                       &aScPreviewShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Margins" ),
     // Slot Nr. 35 : 26516
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREVIEW_SCALINGFACTOR,SfxGroupId::Format,
                       &aScPreviewShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxZoomSliderItem,
                       31/*Offset*/, 0, SfxSlotMode::NONE,"ScalingFactor" ),
     // Slot Nr. 36 : 26543
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_NEXT_TABLE,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToNextTable" ),
     // Slot Nr. 37 : 26544
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_PREV_TABLE,SfxGroupId::Intern,
                       &aScPreviewShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToPrevTable" ),
     // Slot Nr. 38 : 26557
     SFX_NEW_SLOT_ARG( ScPreviewShell,SID_CANCEL,SfxGroupId::Edit,
                       &aScPreviewShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScPreviewShell,Execute),SFX_STUB_PTR(ScPreviewShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       31/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cancel" )
    };
#endif

/************************************************************/
#ifdef ShellClass_Auditing
#undef ShellClass
#undef ShellClass_Auditing
#define ShellClass Auditing
#endif

/************************************************************/
#ifdef ShellClass_ScAuditingShell
#undef ShellClass
#undef ShellClass_ScAuditingShell
#define ShellClass ScAuditingShell
static SfxFormalArgument aScAuditingShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxInt16Item_Impl, "PosX", SID_RANGE_COL },
     { (const SfxType*) &aSfxInt32Item_Impl, "PosY", SID_RANGE_ROW }
    };

SFX_EXEC_STUB(ScAuditingShell,Execute)
SFX_STATE_STUB(ScAuditingShell,GetState)

static SfxSlot aScAuditingShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 26474
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_FILL_ADD_PRED,SfxGroupId::Options,
                       &aScAuditingShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillModeTracePredescessor" ),
     // Slot Nr. 1 : 26475
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_FILL_DEL_PRED,SfxGroupId::Options,
                       &aScAuditingShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillModeRemovePredescessor" ),
     // Slot Nr. 2 : 26476
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_FILL_ADD_SUCC,SfxGroupId::Options,
                       &aScAuditingShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillModeTraceSuccessor" ),
     // Slot Nr. 3 : 26477
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_FILL_DEL_SUCC,SfxGroupId::Options,
                       &aScAuditingShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillModeRemoveSuccessor" ),
     // Slot Nr. 4 : 26478
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_FILL_NONE,SfxGroupId::Edit,
                       &aScAuditingShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillModeEnd" ),
     // Slot Nr. 5 : 26479
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_FILL_SELECT,SfxGroupId::Edit,
                       &aScAuditingShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillModeSelect" ),
     // Slot Nr. 6 : 26557
     SFX_NEW_SLOT_ARG( ScAuditingShell,SID_CANCEL,SfxGroupId::Edit,
                       &aScAuditingShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScAuditingShell,Execute),SFX_STUB_PTR(ScAuditingShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cancel" )
    };
#endif

/************************************************************/
#ifdef ShellClass_ScDrawFormShell
#undef ShellClass
#undef ShellClass_ScDrawFormShell
#define ShellClass ScDrawFormShell
static SfxFormalArgument aScDrawFormShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };


static SfxSlot aScDrawFormShellSlots_Impl[] =
    {
     SFX_SLOT_ARG(ScDrawFormShell, 0, SfxGroupId::NONE, SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,SfxSlotMode::NONE, SfxVoidItem, 0, 0, "", SfxSlotMode::NONE )

    };
#endif

/************************************************************/
#ifdef ShellClass_TableFont
#undef ShellClass
#undef ShellClass_TableFont
#define ShellClass TableFont
#endif

/************************************************************/
#ifdef ShellClass_FormatForSelection
#undef ShellClass
#undef ShellClass_FormatForSelection
#define ShellClass FormatForSelection
#endif

/************************************************************/
#ifdef ShellClass_ScFormatShell
#undef ShellClass
#undef ShellClass_ScFormatShell
#define ShellClass ScFormatShell
static SfxFormalArgument aScFormatShellArgs_Impl[] =
    {
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
     { (const SfxType*) &aSfxStringItem_Impl, "ParamName", SID_STYLE_PREVIEW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "FamilyType", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_CLASSIFICATION_APPLY },
     { (const SfxType*) &aSfxStringItem_Impl, "Type", SID_TYPE_NAME },
     { (const SfxType*) &aSfxBoolItem_Impl, "PersistentCopy", SID_FORMATPAINTBRUSH },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_HIDE },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Param", SID_STYLE_SHOW },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Family", SID_STYLE_FAMILY },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxColorItem_Impl, "Color", SID_ATTR_CHAR_COLOR },
     { (const SfxType*) &aSfxStringItem_Impl, "Color", SID_ATTR_COLOR_STR },
     { (const SfxType*) &aSvxColorItem_Impl, "BackgroundColor", SID_BACKGROUND_COLOR },
     { (const SfxType*) &aSvxBoxItem_Impl, "OuterBorder", SID_ATTR_BORDER_OUTER },
     { (const SfxType*) &aSvxBoxInfoItem_Impl, "InnerBorder", SID_ATTR_BORDER_INNER },
     { (const SfxType*) &aSfxUInt32Item_Impl, "NumberFormatCurrency", SID_NUMBER_CURRENCY }
    };

SFX_EXEC_STUB(ScFormatShell,ExecuteStyle)
SFX_STATE_STUB(ScFormatShell,GetStyleState)
SFX_EXEC_STUB(ScFormatShell,ExecFormatPaintbrush)
SFX_STATE_STUB(ScFormatShell,StateFormatPaintbrush)
SFX_EXEC_STUB(ScFormatShell,ExecuteAttr)
SFX_STATE_STUB(ScFormatShell,GetAttrState)
SFX_EXEC_STUB(ScFormatShell,ExecuteTextAttr)
SFX_STATE_STUB(ScFormatShell,GetTextAttrState)
SFX_STATE_STUB(ScFormatShell,GetBorderState)
SFX_EXEC_STUB(ScFormatShell,ExecuteNumFormat)
SFX_STATE_STUB(ScFormatShell,GetNumFormatState)
SFX_EXEC_STUB(ScFormatShell,ExecuteAlignment)
SFX_EXEC_STUB(ScFormatShell,ExecuteTextDirection)
SFX_STATE_STUB(ScFormatShell,GetTextDirectionState)
SFX_STATE_STUB(ScFormatShell,GetAlignState)

static SfxSlot aScFormatShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5542
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_FAMILY2,SfxGroupId::Document,
                       &aScFormatShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"ParaStyle" ),
     // Slot Nr. 1 : 5544
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_FAMILY4,SfxGroupId::Document,
                       &aScFormatShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"PageStyle" ),
     // Slot Nr. 2 : 5549
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_NEW,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NewStyle" ),
     // Slot Nr. 3 : 5550
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_EDIT,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditStyle" ),
     // Slot Nr. 4 : 5551
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_DELETE,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteStyle" ),
     // Slot Nr. 5 : 5552
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_APPLY,SfxGroupId::Document,
                       &aScFormatShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxTemplateItem,
                       6/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleApply" ),
     // Slot Nr. 6 : 5554
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_WATERCAN,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       10/*Offset*/, 0, SfxSlotMode::NONE,"StyleWatercanMode" ),
     // Slot Nr. 7 : 5555
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_NEW_BY_EXAMPLE,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       10/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleNewByExample" ),
     // Slot Nr. 8 : 5556
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_UPDATE_BY_EXAMPLE,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       12/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StyleUpdateByExample" ),
     // Slot Nr. 9 : 5567
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_PREVIEW,SfxGroupId::NONE,
                       &aScFormatShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::NORECORD|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       14/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StylePreview" ),
     // Slot Nr. 10 : 5568
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_END_PREVIEW,SfxGroupId::NONE,
                       &aScFormatShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::NORECORD|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 0, SfxSlotMode::NONE,"StyleEndPreview" ),
     // Slot Nr. 11 : 5672
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_CLASSIFICATION_APPLY,SfxGroupId::Document,
                       &aScFormatShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       16/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClassificationApply" ),
     // Slot Nr. 12 : 5715
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_FORMATPAINTBRUSH,SfxGroupId::Edit,
                       &aScFormatShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecFormatPaintbrush),SFX_STUB_PTR(ScFormatShell,StateFormatPaintbrush),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       18/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatPaintbrush" ),
     // Slot Nr. 13 : 6603
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_HIDE,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideStyle" ),
     // Slot Nr. 14 : 6604
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_STYLE_SHOW,SfxGroupId::Template,
                       &aScFormatShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteStyle),SFX_STUB_PTR(ScFormatShell,GetStyleState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowStyle" ),
     // Slot Nr. 15 : 10001
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BRUSH,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBrushItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"BackgroundPattern" ),
     // Slot Nr. 16 : 10007
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_FONT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"CharFontName" ),
     // Slot Nr. 17 : 10008
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_POSTURE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxPostureItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Italic" ),
     // Slot Nr. 18 : 10009
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_WEIGHT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxWeightItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Bold" ),
     // Slot Nr. 19 : 10010
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_SHADOWED,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxShadowedItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Shadowed" ),
     // Slot Nr. 20 : 10012
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_CONTOUR,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxContourItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"OutlineFont" ),
     // Slot Nr. 21 : 10013
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_STRIKEOUT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCrossedOutItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Strikeout" ),
     // Slot Nr. 22 : 10014
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_UNDERLINE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxUnderlineItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"Underline" ),
     // Slot Nr. 23 : 10015
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_FONTHEIGHT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontHeightItem,
                       23/*Offset*/, 0, SfxSlotMode::NONE,"FontHeight" ),
     // Slot Nr. 24 : 10017
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_COLOR,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       23/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Color" ),
     // Slot Nr. 25 : 10023
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BORDER_INNER,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScFormatShell,GetBorderState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBoxInfoItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"BorderInner" ),
     // Slot Nr. 26 : 10024
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BORDER_OUTER,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetBorderState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBoxItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"BorderOuter" ),
     // Slot Nr. 27 : 10025
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BORDER_SHADOW,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetBorderState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxShadowItem,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"BorderShadow" ),
     // Slot Nr. 28 : 10085
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_NUMBERFORMAT_VALUE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       25/*Offset*/, 0, SfxSlotMode::NONE,"NumberFormatValue" ),
     // Slot Nr. 29 : 10186
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_BACKGROUND_COLOR,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       25/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"BackgroundColor" ),
     // Slot Nr. 30 : 10188
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BORDER,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxBoxItem,
                       27/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetBorderStyle" ),
     // Slot Nr. 31 : 10201
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_FRAME_LINESTYLE,SfxGroupId::Frame,
                       &aScFormatShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"LineStyle" ),
     // Slot Nr. 32 : 10202
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_FRAME_LINECOLOR,SfxGroupId::Frame,
                       &aScFormatShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxColorItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"FrameLineColor" ),
     // Slot Nr. 33 : 10228
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_MARGIN,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxMarginItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"AlignmentMargin" ),
     // Slot Nr. 34 : 10229
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_STACKED,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"AlignmentStacked" ),
     // Slot Nr. 35 : 10230
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_LINEBREAK,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"WrapText" ),
     // Slot Nr. 36 : 10460
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_INDENT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"AlignmentIndent" ),
     // Slot Nr. 37 : 10577
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_DEGREES,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"AlignmentRotation" ),
     // Slot Nr. 38 : 10578
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_LOCKPOS,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxRotateModeItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"AlignmentRotationMode" ),
     // Slot Nr. 39 : 10907
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_TEXTDIRECTION_LEFT_TO_RIGHT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextDirection),SFX_STUB_PTR(ScFormatShell,GetTextDirectionState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TextdirectionLeftToRight" ),
     // Slot Nr. 40 : 10908
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_TEXTDIRECTION_TOP_TO_BOTTOM,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextDirection),SFX_STUB_PTR(ScFormatShell,GetTextDirectionState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"TextdirectionTopToBottom" ),
     // Slot Nr. 41 : 10920
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_RELIEF,SfxGroupId::Controls,
                       &aScFormatShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxCharReliefItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"CharacterRelief" ),
     // Slot Nr. 42 : 10931
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_HYPHENATION,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"AlignmentHyphenation" ),
     // Slot Nr. 43 : 10950
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_PARA_LEFT_TO_RIGHT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextDirection),SFX_STUB_PTR(ScFormatShell,GetTextDirectionState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"ParaLeftToRight" ),
     // Slot Nr. 44 : 10951
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_PARA_RIGHT_TO_LEFT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextDirection),SFX_STUB_PTR(ScFormatShell,GetTextDirectionState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"ParaRightToLeft" ),
     // Slot Nr. 45 : 10956
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_CTLFONT_STATE,SfxGroupId::Application,
                       &aScFormatShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScFormatShell,GetTextDirectionState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"CTLFontState" ),
     // Slot Nr. 46 : 10957
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_VERTICALTEXT_STATE,SfxGroupId::Application,
                       &aScFormatShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScFormatShell,GetTextDirectionState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"VerticalTextState" ),
     // Slot Nr. 47 : 11002
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_LEFT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignLeft" ),
     // Slot Nr. 48 : 11003
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_HCENTER,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignHorizontalCenter" ),
     // Slot Nr. 49 : 11004
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_RIGHT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignRight" ),
     // Slot Nr. 50 : 11005
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_JUSTIFIED,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignJustified" ),
     // Slot Nr. 51 : 11006
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_TOP,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignTop" ),
     // Slot Nr. 52 : 11007
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_VCENTER,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignVerticalCenter" ),
     // Slot Nr. 53 : 11008
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_BOTTOM,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignBottom" ),
     // Slot Nr. 54 : 11009
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_HDEFAULT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignHorizontalDefault" ),
     // Slot Nr. 55 : 11010
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGN_ANY_VDEFAULT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CommonAlignVerticalDefault" ),
     // Slot Nr. 56 : 11013
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BORDER_DIAG_TLBR,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"BorderTLBR" ),
     // Slot Nr. 57 : 11014
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_BORDER_DIAG_BLTR,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxLineItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"BorderBLTR" ),
     // Slot Nr. 58 : 11042
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_GROW_FONT_SIZE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Grow" ),
     // Slot Nr. 59 : 11043
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_SHRINK_FONT_SIZE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       29/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Shrink" ),
     // Slot Nr. 60 : 11195
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ULINE_VAL_NONE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineNone" ),
     // Slot Nr. 61 : 11196
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ULINE_VAL_SINGLE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineSingle" ),
     // Slot Nr. 62 : 11197
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ULINE_VAL_DOUBLE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDouble" ),
     // Slot Nr. 63 : 11198
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ULINE_VAL_DOTTED,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"UnderlineDotted" ),
     // Slot Nr. 64 : 11568
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_OVERLINE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxOverlineItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"Overline" ),
     // Slot Nr. 65 : 11571
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_HOR_JUSTIFY,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHorJustifyItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"HorizontalJustification" ),
     // Slot Nr. 66 : 11572
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_ALIGN_VER_JUSTIFY,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxVerJustifyItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"VerticalJustification" ),
     // Slot Nr. 67 : 11574
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_PREVIEW_FONT,SfxGroupId::NONE,
                       &aScFormatShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::NORECORD|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"CharPreviewFontName" ),
     // Slot Nr. 68 : 11575
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ATTR_CHAR_ENDPREVIEW_FONT,SfxGroupId::NONE,
                       &aScFormatShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::NORECORD|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxFontItem,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"CharEndPreviewFontName" ),
     // Slot Nr. 69 : 26036
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_SCATTR_PROTECTION,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       ScProtectionAttr,
                       29/*Offset*/, 0, SfxSlotMode::NONE,"Protection" ),
     // Slot Nr. 70 : 26045
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_CURRENCY,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       29/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatCurrency" ),
     // Slot Nr. 71 : 26046
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_PERCENT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatPercent" ),
     // Slot Nr. 72 : 26052
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_STANDARD,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatStandard" ),
     // Slot Nr. 73 : 26053
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_DATE,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatDate" ),
     // Slot Nr. 74 : 26054
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_TWODEC,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatDecimal" ),
     // Slot Nr. 75 : 26055
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_SCIENTIFIC,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatScientific" ),
     // Slot Nr. 76 : 26056
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_TIME,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatTime" ),
     // Slot Nr. 77 : 26057
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_INCDEC,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatIncDecimals" ),
     // Slot Nr. 78 : 26058
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_DECDEC,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatDecDecimals" ),
     // Slot Nr. 79 : 26059
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_THOUSANDS,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"NumberFormatThousands" ),
     // Slot Nr. 80 : 26382
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNLEFT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignLeft" ),
     // Slot Nr. 81 : 26383
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNRIGHT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignRight" ),
     // Slot Nr. 82 : 26384
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNCENTERHOR,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignHorizontalCenter" ),
     // Slot Nr. 83 : 26385
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNBLOCK,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignBlock" ),
     // Slot Nr. 84 : 26386
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNTOP,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignTop" ),
     // Slot Nr. 85 : 26387
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNBOTTOM,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignBottom" ),
     // Slot Nr. 86 : 26388
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_ALIGNCENTERVER,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteTextAttr),SFX_STUB_PTR(ScFormatShell,GetTextAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       30/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AlignVCenter" ),
     // Slot Nr. 87 : 26390
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_V_ALIGNCELL,SfxGroupId::Intern,
                       &aScFormatShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxVerJustifyItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"VerticalAlignment" ),
     // Slot Nr. 88 : 26391
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_H_ALIGNCELL,SfxGroupId::View,
                       &aScFormatShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAlignment),SFX_STUB_PTR(ScFormatShell,GetAlignState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHorJustifyItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"HorizontalAlignment" ),
     // Slot Nr. 89 : 26401
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_TYPE_FORMAT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"NumberFormatType" ),
     // Slot Nr. 90 : 26620
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_NUMBER_FORMAT,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteNumFormat),SFX_STUB_PTR(ScFormatShell,GetNumFormatState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"NumberFormat" ),
     // Slot Nr. 91 : 26676
     SFX_NEW_SLOT_ARG( ScFormatShell,SID_SCATTR_CELLPROTECTION,SfxGroupId::Format,
                       &aScFormatShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScFormatShell,ExecuteAttr),SFX_STUB_PTR(ScFormatShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       30/*Offset*/, 0, SfxSlotMode::NONE,"CellProtection" )
    };
#endif

/************************************************************/
#ifdef ShellClass_CellSelection
#undef ShellClass
#undef ShellClass_CellSelection
#define ShellClass CellSelection
#endif

/************************************************************/
#ifdef ShellClass_CellMovement
#undef ShellClass
#undef ShellClass_CellMovement
#define ShellClass CellMovement
#endif

/************************************************************/
#ifdef ShellClass_Cell
#undef ShellClass
#undef ShellClass_Cell
#define ShellClass Cell
#endif

/************************************************************/
#ifdef ShellClass_ScCellShell
#undef ShellClass
#undef ShellClass_ScCellShell
#define ShellClass ScCellShell
static SfxFormalArgument aScCellShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxUInt32Item_Impl, "Format", SID_PASTE_SPECIAL },
     { (const SfxType*) &aSfxUInt32Item_Impl, "SelectedFormat", SID_CLIPBOARD_FORMAT_ITEMS },
     { (const SfxType*) &aSfxUInt16Item_Impl, "AnchorType", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IgnoreComments", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Flags", SID_DELETE },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "aFormatName", SID_AUTOFORMAT },
     { (const SfxType*) &aSvxHyperlinkItem_Impl, "Hyperlink", SID_HYPERLINK_SETLINK },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbols", SID_CHARMAP },
     { (const SfxType*) &aSfxStringItem_Impl, "FontName", SID_ATTR_SPECIALCHAR },
     { (const SfxType*) &aSvxPostItAuthorItem_Impl, "Author", SID_ATTR_POSTIT_AUTHOR },
     { (const SfxType*) &aSvxPostItDateItem_Impl, "Date", SID_ATTR_POSTIT_DATE },
     { (const SfxType*) &aSvxPostItTextItem_Impl, "Text", SID_ATTR_POSTIT_TEXT },
     { (const SfxType*) &aSvxPostItIdItem_Impl, "Id", SID_ATTR_POSTIT_ID },
     { (const SfxType*) &aSvxPostItAuthorItem_Impl, "Author", SID_ATTR_POSTIT_AUTHOR },
     { (const SfxType*) &aSvxPostItDateItem_Impl, "Date", SID_ATTR_POSTIT_DATE },
     { (const SfxType*) &aSvxPostItTextItem_Impl, "Text", SID_ATTR_POSTIT_TEXT },
     { (const SfxType*) &aSfxBoolItem_Impl, "AllCharts", SID_UPDATECHART },
     { (const SfxType*) &aSfxStringItem_Impl, "FileName", SID_FILE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "FilterName", SID_FILTER_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Options", SID_FILE_FILTEROPTIONS },
     { (const SfxType*) &aSfxStringItem_Impl, "Source", FN_PARAM_1 },
     { (const SfxType*) &aSfxUInt32Item_Impl, "Refresh", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "StringName", SID_ENTER_STRING },
     { (const SfxType*) &aSfxBoolItem_Impl, "DontCommit", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "IconSet", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Visible", SID_SEARCH_RESULTS_DIALOG },
     { (const SfxType*) &aSfxStringItem_Impl, "Flags", FID_DELETE_CELL },
     { (const SfxType*) &aSfxStringItem_Impl, "Flags", FID_FILL_TAB },
     { (const SfxType*) &aSfxStringItem_Impl, "FillDir", FID_FILL_SERIES },
     { (const SfxType*) &aSfxStringItem_Impl, "FillCmd", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "FillStep", FN_PARAM_3 },
     { (const SfxType*) &aSfxStringItem_Impl, "FillDateCmd", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "FillStart", FN_PARAM_4 },
     { (const SfxType*) &aSfxStringItem_Impl, "FillMax", FN_PARAM_5 },
     { (const SfxType*) &aSfxStringItem_Impl, "Flags", FID_INS_CELL_CONTENTS },
     { (const SfxType*) &aSfxUInt16Item_Impl, "FormulaCommand", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "SkipEmptyCells", FN_PARAM_2 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Transpose", FN_PARAM_3 },
     { (const SfxType*) &aSfxBoolItem_Impl, "AsLink", FN_PARAM_4 },
     { (const SfxType*) &aSfxInt16Item_Impl, "MoveMode", FN_PARAM_5 },
     { (const SfxType*) &aSfxStringItem_Impl, "Flags", FID_INS_CELL },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_DEFINE_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbol", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Options", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", FID_DEFINE_CURRENT_NAME },
     { (const SfxType*) &aSfxStringItem_Impl, "Symbol", FN_PARAM_1 },
     { (const SfxType*) &aSfxStringItem_Impl, "Options", FN_PARAM_2 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "RowHeight", FID_ROW_HEIGHT },
     { (const SfxType*) &aSfxInt32Item_Impl, "Row", FN_PARAM_1 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "aExtraHeight", FID_ROW_OPT_HEIGHT },
     { (const SfxType*) &aSfxUInt16Item_Impl, "ColumnWidth", FID_COL_WIDTH },
     { (const SfxType*) &aSfxUInt16Item_Impl, "Column", FN_PARAM_1 },
     { (const SfxType*) &aSfxUInt16Item_Impl, "aExtraWidth", FID_COL_OPT_WIDTH },
     { (const SfxType*) &aSfxBoolItem_Impl, "MoveContents", FID_MERGE_ON },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_SCENARIOS },
     { (const SfxType*) &aSfxStringItem_Impl, "Comment", SID_NEW_TABLENAME },
     { (const SfxType*) &aSfxStringItem_Impl, "DbName", SID_SELECT_DB },
     { (const SfxType*) &aSfxBoolItem_Impl, "ByRows", SID_SORT_BYROW },
     { (const SfxType*) &aSfxBoolItem_Impl, "HasHeader", SID_SORT_HASHEADER },
     { (const SfxType*) &aSfxBoolItem_Impl, "CaseSensitive", SID_SORT_CASESENS },
     { (const SfxType*) &aSfxBoolItem_Impl, "NaturalSort", SID_SORT_NATURALSORT },
     { (const SfxType*) &aSfxBoolItem_Impl, "IncludeAttribs", SID_SORT_ATTRIBS },
     { (const SfxType*) &aSfxUInt16Item_Impl, "UserDefIndex", SID_SORT_USERDEF },
     { (const SfxType*) &aSfxInt32Item_Impl, "Col1", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Ascending1", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt32Item_Impl, "Col2", FN_PARAM_3 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Ascending2", FN_PARAM_4 },
     { (const SfxType*) &aSfxInt32Item_Impl, "Col3", FN_PARAM_5 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Ascending3", FN_PARAM_6 },
     { (const SfxType*) &aSfxBoolItem_Impl, "IncludeComments", SID_SORT_INCCOMMENTS },
     { (const SfxType*) &aSfxBoolItem_Impl, "IncludeImages", SID_SORT_INCIMAGES },
     { (const SfxType*) &aSfxStringItem_Impl, "DbName", SID_MANAGE_XML_SOURCE },
     { (const SfxType*) &aSfxStringItem_Impl, "RowOrCol", SID_OUTLINE_MAKE },
     { (const SfxType*) &aSfxStringItem_Impl, "RowOrCol", SID_OUTLINE_REMOVE },
     { (const SfxType*) &aSfxStringItem_Impl, "Name", SID_SELECT_SCENARIO },
     { (const SfxType*) &aSfxStringItem_Impl, "Formula", SID_INSERT_MATRIX },
     { (const SfxType*) &aSvxPostItIdItem_Impl, "Id", SID_ATTR_POSTIT_ID },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxBoolItem_Impl, "Sel", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "By", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt32Item_Impl, "Col", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "Modifier", FN_PARAM_2 },
     { (const SfxType*) &aSfxInt32Item_Impl, "Row", FN_PARAM_1 },
     { (const SfxType*) &aSfxInt16Item_Impl, "Modifier", FN_PARAM_2 },
     { (const SfxType*) &aSfxStringItem_Impl, "EndCell", FID_FILL_AUTO },
     { (const SfxType*) &aSfxBoolItem_Impl, "MoveContents", FID_MERGE_TOGGLE }
    };

SFX_EXEC_STUB(ScCellShell,ExecuteEdit)
SFX_STATE_STUB(ScCellShell,GetClipState)
SFX_STATE_STUB(ScCellShell,GetCellState)
SFX_STATE_STUB(ScCellShell,GetBlockState)
SFX_EXEC_STUB(ScCellShell,Execute)
SFX_STATE_STUB(ScCellShell,GetState)
SFX_EXEC_STUB(ScCellShell,ExecuteCursor)
SFX_STATE_STUB(ScCellShell,GetStateCursor)
SFX_EXEC_STUB(ScCellShell,ExecutePage)
SFX_EXEC_STUB(ScCellShell,ExecuteMove)
SFX_EXEC_STUB(ScCellShell,ExecuteDB)
SFX_STATE_STUB(ScCellShell,GetDBState)
SFX_STATE_STUB(ScCellShell,GetHLinkState)
SFX_EXEC_STUB(ScCellShell,ExecuteTrans)
SFX_EXEC_STUB(ScCellShell,ExecuteRotateTrans)
SFX_EXEC_STUB(ScCellShell,ExecuteCursorSel)
SFX_EXEC_STUB(ScCellShell,ExecutePageSel)

static SfxSlot aScCellShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 5311
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PASTE_SPECIAL,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteSpecial" ),
     // Slot Nr. 1 : 5312
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CLIPBOARD_FORMAT_ITEMS,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxClipboardFormatItem,
                       1/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClipboardFormatItems" ),
     // Slot Nr. 2 : 5314
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PASTE_UNFORMATTED,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"PasteUnformatted" ),
     // Slot Nr. 3 : 5405
     SFX_NEW_SLOT_ARG( ScCellShell,SID_EMOJI_CONTROL,SfxGroupId::Special,
                       &aScCellShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"EmojiControl" ),
     // Slot Nr. 4 : 5406
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CHARMAP_CONTROL,SfxGroupId::Special,
                       &aScCellShellSlots_Impl[31] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0, SfxSlotMode::NONE,"CharmapControl" ),
     // Slot Nr. 5 : 5710
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CUT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cut" ),
     // Slot Nr. 6 : 5711
     SFX_NEW_SLOT_ARG( ScCellShell,SID_COPY,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Copy" ),
     // Slot Nr. 7 : 5712
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PASTE,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       2/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Paste" ),
     // Slot Nr. 8 : 5713
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DELETE,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[29] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       4/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Delete" ),
     // Slot Nr. 9 : 5723
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECTALL,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectAll" ),
     // Slot Nr. 10 : 5731
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORDOWN,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       5/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDown" ),
     // Slot Nr. 11 : 5732
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORUP,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       7/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUp" ),
     // Slot Nr. 12 : 5733
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORLEFT,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       9/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeft" ),
     // Slot Nr. 13 : 5734
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORRIGHT,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       11/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRight" ),
     // Slot Nr. 14 : 5735
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGEDOWN,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       13/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDownBlock" ),
     // Slot Nr. 15 : 5736
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGEUP,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       15/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUpBlock" ),
     // Slot Nr. 16 : 5741
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORENDOFFILE,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePage),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       17/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToEndOfData" ),
     // Slot Nr. 17 : 5742
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORTOPOFFILE,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePage),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       18/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToStart" ),
     // Slot Nr. 18 : 5743
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORENDOFSCREEN,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CursorEndOfScreen" ),
     // Slot Nr. 19 : 5744
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORTOPOFSCREEN,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CursorTopOfScreen" ),
     // Slot Nr. 20 : 5745
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORHOME,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePage),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       19/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToStartOfRow" ),
     // Slot Nr. 21 : 5746
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSOREND,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[67] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePage),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       20/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToEndOfRow" ),
     // Slot Nr. 22 : 5802
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PASTE_ONLY_TEXT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"PasteOnlyText" ),
     // Slot Nr. 23 : 5803
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PASTE_ONLY_FORMULA,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"PasteOnlyFormula" ),
     // Slot Nr. 24 : 5804
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PASTE_ONLY_VALUE,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetClipState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"PasteOnlyValue" ),
     // Slot Nr. 25 : 6660
     SFX_NEW_SLOT_ARG( ScCellShell,SID_VIEW_DATA_SOURCE_BROWSER,SfxGroupId::View,
                       &aScCellShellSlots_Impl[54] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"ViewDataSourceBrowser" ),
     // Slot Nr. 26 : 10223
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ATTR_POSITION,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[27] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxPointItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"Position" ),
     // Slot Nr. 27 : 10224
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ATTR_SIZE,SfxGroupId::View,
                       &aScCellShellSlots_Impl[28] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxSizeItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"Size" ),
     // Slot Nr. 28 : 10225
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TABLE_CELL,SfxGroupId::View,
                       &aScCellShellSlots_Impl[30] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       21/*Offset*/, 0, SfxSlotMode::NONE,"StateTableCell" ),
     // Slot Nr. 29 : 10242
     SFX_NEW_SLOT_ARG( ScCellShell,SID_AUTOFORMAT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[37] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       21/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AutoFormat" ),
     // Slot Nr. 30 : 10243
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SPELL_DIALOG,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[34] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       22/*Offset*/, 0, SfxSlotMode::NONE,"SpellDialog" ),
     // Slot Nr. 31 : 10245
     SFX_NEW_SLOT_ARG( ScCellShell,SID_THESAURUS,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[36] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ThesaurusDialog" ),
     // Slot Nr. 32 : 10361
     SFX_NEW_SLOT_ARG( ScCellShell,SID_HYPERLINK_GETLINK,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[32] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetHLinkState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SvxHyperlinkItem,
                       22/*Offset*/, 0, SfxSlotMode::NONE,"Hyperlink" ),
     // Slot Nr. 33 : 10362
     SFX_NEW_SLOT_ARG( ScCellShell,SID_HYPERLINK_SETLINK,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[51] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       22/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetHyperlink" ),
     // Slot Nr. 34 : 10461
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DEC_INDENT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[35] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       23/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DecrementIndent" ),
     // Slot Nr. 35 : 10462
     SFX_NEW_SLOT_ARG( ScCellShell,SID_INC_INDENT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[43] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       23/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"IncrementIndent" ),
     // Slot Nr. 36 : 10503
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CHARMAP,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[46] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       23/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertSymbol" ),
     // Slot Nr. 37 : 10912
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_UPPER,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[38] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToUpper" ),
     // Slot Nr. 38 : 10913
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_LOWER,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[39] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToLower" ),
     // Slot Nr. 39 : 10914
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_HALFWIDTH,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[40] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHalfWidth" ),
     // Slot Nr. 40 : 10915
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_FULLWIDTH,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[41] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToFullWidth" ),
     // Slot Nr. 41 : 10916
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_HIRAGANA,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[42] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToHiragana" ),
     // Slot Nr. 42 : 10917
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_KATAKANA,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[47] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToKatakana" ),
     // Slot Nr. 43 : 10959
     SFX_NEW_SLOT_ARG( ScCellShell,SID_HANGUL_HANJA_CONVERSION,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[44] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HangulHanjaConversion" ),
     // Slot Nr. 44 : 11016
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CHINESE_CONVERSION,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[45] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChineseConversion" ),
     // Slot Nr. 45 : 11098
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TOGGLE_NOTES,SfxGroupId::View,
                       &aScCellShellSlots_Impl[56] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowAnnotations" ),
     // Slot Nr. 46 : 11101
     SFX_NEW_SLOT_ARG( ScCellShell,SID_INSERT_POSTIT,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[53] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       25/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertAnnotation" ),
     // Slot Nr. 47 : 11102
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_SENTENCE_CASE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[48] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToSentenceCase" ),
     // Slot Nr. 48 : 11103
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_TITLE_CASE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[49] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToTitleCase" ),
     // Slot Nr. 49 : 11104
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_TOGGLE_CASE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[50] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseToToggleCase" ),
     // Slot Nr. 50 : 11105
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TRANSLITERATE_ROTATE_CASE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[64] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteRotateTrans),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangeCaseRotateCase" ),
     // Slot Nr. 51 : 11132
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CELL_FORMAT_BORDER,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[52] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatCellBorders" ),
     // Slot Nr. 52 : 11133
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CHAR_DLG_EFFECT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[75] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FontEffectsDialog" ),
     // Slot Nr. 53 : 11158
     SFX_NEW_SLOT_ARG( ScCellShell,SID_EDIT_POSTIT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[85] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       28/*Offset*/, 4/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EditAnnotation" ),
     // Slot Nr. 54 : 12202
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SBA_BRW_INSERT,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[55] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SbaBrwInsert" ),
     // Slot Nr. 55 : 25015
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_FORM,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[59] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataForm" ),
     // Slot Nr. 56 : 25016
     SFX_NEW_SLOT_ARG( ScCellShell,SID_INSERT_CURRENT_DATE,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[57] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertCurrentDate" ),
     // Slot Nr. 57 : 25017
     SFX_NEW_SLOT_ARG( ScCellShell,SID_INSERT_CURRENT_TIME,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[58] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertCurrentTime" ),
     // Slot Nr. 58 : 26013
     SFX_NEW_SLOT_ARG( ScCellShell,SID_UPDATECHART,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[68] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       32/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"UpdateChart" ),
     // Slot Nr. 59 : 26031
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_PROVIDER,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[60] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataProvider" ),
     // Slot Nr. 60 : 26032
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_PROVIDER_REFRESH,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[61] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataProviderRefresh" ),
     // Slot Nr. 61 : 26033
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_STREAMS_PLAY,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[62] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0, SfxSlotMode::NONE,"DataStreamsPlay" ),
     // Slot Nr. 62 : 26034
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_STREAMS_STOP,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[63] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0, SfxSlotMode::NONE,"DataStreamsStop" ),
     // Slot Nr. 63 : 26035
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_STREAMS,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[71] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataStreams" ),
     // Slot Nr. 64 : 26067
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CELL_FORMAT_RESET,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[65] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ResetAttributes" ),
     // Slot Nr. 65 : 26085
     SFX_NEW_SLOT_ARG( ScCellShell,SID_EXTERNAL_SOURCE,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[66] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       33/*Offset*/, 5/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertExternalDataSource" ),
     // Slot Nr. 66 : 26087
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ENABLE_HYPHENATION,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[87] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Hyphenate" ),
     // Slot Nr. 67 : 26089
     SFX_NEW_SLOT_ARG( ScCellShell,SID_FOCUS_INPUTLINE,SfxGroupId::Application,
                       &aScCellShellSlots_Impl[195] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::CONTAINER|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FocusInputLine" ),
     // Slot Nr. 68 : 26100
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INPUTLINE_STATUS,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[69] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"StatusInputLine" ),
     // Slot Nr. 69 : 26101
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INPUTLINE_ENTER,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[70] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::NORECORD|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InputLineEnter" ),
     // Slot Nr. 70 : 26106
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SOLVE,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[72] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoalSeek" ),
     // Slot Nr. 71 : 26108
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILTER_OK,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[84] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FilterExecute" ),
     // Slot Nr. 72 : 26111
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INPUTLINE_MATRIX,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[73] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InputLineMatrix" ),
     // Slot Nr. 73 : 26114
     SFX_NEW_SLOT_ARG( ScCellShell,SID_STATUS_DOCPOS,SfxGroupId::View,
                       &aScCellShellSlots_Impl[74] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"StatusDocPos" ),
     // Slot Nr. 74 : 26116
     SFX_NEW_SLOT_ARG( ScCellShell,SID_STATUS_SELMODE,SfxGroupId::View,
                       &aScCellShellSlots_Impl[78] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"StatusSelectionMode" ),
     // Slot Nr. 75 : 26122
     SFX_NEW_SLOT_ARG( ScCellShell,SID_STATUS_SELMODE_ERG,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[76] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"StatusSelectionModeExp" ),
     // Slot Nr. 76 : 26123
     SFX_NEW_SLOT_ARG( ScCellShell,SID_STATUS_SELMODE_ERW,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[77] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"StatusSelectionModeExt" ),
     // Slot Nr. 77 : 26124
     SFX_NEW_SLOT_ARG( ScCellShell,SID_STATUS_SELMODE_NORM,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[82] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"StatusSelectionModeNorm" ),
     // Slot Nr. 78 : 26129
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INPUTLINE_BLOCK,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[79] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InputLineBlock" ),
     // Slot Nr. 79 : 26130
     SFX_NEW_SLOT_ARG( ScCellShell,SID_STATUS_SUM,SfxGroupId::View,
                       &aScCellShellSlots_Impl[80] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       38/*Offset*/, 0, SfxSlotMode::NONE,"StatusFunction" ),
     // Slot Nr. 80 : 26137
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ENTER_STRING,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[81] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       38/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"EnterString" ),
     // Slot Nr. 81 : 26138
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ROWCOL_SELCOUNT,SfxGroupId::View,
                       &aScCellShellSlots_Impl[83] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"RowColSelCount" ),
     // Slot Nr. 82 : 26139
     SFX_NEW_SLOT_ARG( ScCellShell,SID_AUTO_SUM,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[89] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"AutoSum" ),
     // Slot Nr. 83 : 26150
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_CONSOLIDATE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[86] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataConsolidate" ),
     // Slot Nr. 84 : 26151
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_PIVOTTABLE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[160] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataDataPilotRun" ),
     // Slot Nr. 85 : 26152
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_FUNCTION,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetCellState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FunctionDialog" ),
     // Slot Nr. 86 : 26153
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_SOLVE,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[88] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoalSeekDialog" ),
     // Slot Nr. 87 : 26154
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_TABOP,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[99] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableOperationDialog" ),
     // Slot Nr. 88 : 26158
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_CURRENTCONDFRMT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[90] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CurrentConditionalFormatDialog" ),
     // Slot Nr. 89 : 26159
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_CONDFRMT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[91] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConditionalFormatDialog" ),
     // Slot Nr. 90 : 26160
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_OPTSOLVER,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[94] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SolverDialog" ),
     // Slot Nr. 91 : 26161
     SFX_NEW_SLOT_ARG( ScCellShell,SID_VALIDITY_REFERENCE,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[92] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ValidityReference" ),
     // Slot Nr. 92 : 26162
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_CONDFRMT_MANAGER,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[93] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConditionalFormatManagerDialog" ),
     // Slot Nr. 93 : 26163
     SFX_NEW_SLOT_ARG( ScCellShell,SID_POPUP_CONDFRMT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[95] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0, SfxSlotMode::NONE,"ConditionalFormatMenu" ),
     // Slot Nr. 94 : 26164
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_CURRENTCONDFRMT_MANAGER,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[122] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CurrentConditionalFormatManagerDialog" ),
     // Slot Nr. 95 : 26166
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_COLORSCALE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[96] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ColorScaleFormatDialog" ),
     // Slot Nr. 96 : 26167
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_DATABAR,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[97] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataBarFormatDialog" ),
     // Slot Nr. 97 : 26168
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_ICONSET,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[98] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       40/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"IconSetFormatDialog" ),
     // Slot Nr. 98 : 26169
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OPENDLG_CONDDATE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[182] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CondDateFormatDialog" ),
     // Slot Nr. 99 : 26170
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANDOM_NUMBER_GENERATOR_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[100] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RandomNumberGeneratorDialog" ),
     // Slot Nr. 100 : 26171
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SAMPLING_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[101] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SamplingDialog" ),
     // Slot Nr. 101 : 26172
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DESCRIPTIVE_STATISTICS_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[102] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DescriptiveStatisticsDialog" ),
     // Slot Nr. 102 : 26173
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ANALYSIS_OF_VARIANCE_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[103] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AnalysisOfVarianceDialog" ),
     // Slot Nr. 103 : 26174
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CORRELATION_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[104] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CorrelationDialog" ),
     // Slot Nr. 104 : 26175
     SFX_NEW_SLOT_ARG( ScCellShell,SID_COVARIANCE_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[105] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CovarianceDialog" ),
     // Slot Nr. 105 : 26176
     SFX_NEW_SLOT_ARG( ScCellShell,SID_EXPONENTIAL_SMOOTHING_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[106] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExponentialSmoothingDialog" ),
     // Slot Nr. 106 : 26177
     SFX_NEW_SLOT_ARG( ScCellShell,SID_MOVING_AVERAGE_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[107] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MovingAverageDialog" ),
     // Slot Nr. 107 : 26178
     SFX_NEW_SLOT_ARG( ScCellShell,SID_REGRESSION_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[108] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RegressionDialog" ),
     // Slot Nr. 108 : 26179
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TTEST_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[109] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TTestDialog" ),
     // Slot Nr. 109 : 26180
     SFX_NEW_SLOT_ARG( ScCellShell,SID_FTEST_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[110] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FTestDialog" ),
     // Slot Nr. 110 : 26181
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ZTEST_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[111] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ZTestDialog" ),
     // Slot Nr. 111 : 26182
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CHI_SQUARE_TEST_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[112] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChiSquareTestDialog" ),
     // Slot Nr. 112 : 26183
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SEARCH_RESULTS_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[113] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       41/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SearchResultsDialog" ),
     // Slot Nr. 113 : 26186
     SFX_NEW_SLOT_ARG( ScCellShell,SID_COLUMN_OPERATIONS,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[114] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       42/*Offset*/, 0, SfxSlotMode::NONE,"ColumnOperations" ),
     // Slot Nr. 114 : 26187
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ROW_OPERATIONS,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[115] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       42/*Offset*/, 0, SfxSlotMode::NONE,"RowOperations" ),
     // Slot Nr. 115 : 26188
     SFX_NEW_SLOT_ARG( ScCellShell,SID_FOURIER_ANALYSIS_DIALOG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[116] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       42/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FourierAnalysisDialog" ),
     // Slot Nr. 116 : 26205
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CONVERT_FORMULA_TO_VALUE,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[117] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       42/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConvertFormulaToValue" ),
     // Slot Nr. 117 : 26222
     SFX_NEW_SLOT_ARG( ScCellShell,FID_DELETE_CELL,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[118] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       42/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteCell" ),
     // Slot Nr. 118 : 26224
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_TO_BOTTOM,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[119] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillDown" ),
     // Slot Nr. 119 : 26225
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_TO_RIGHT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[120] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillRight" ),
     // Slot Nr. 120 : 26226
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_TO_TOP,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[121] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillUp" ),
     // Slot Nr. 121 : 26227
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_TO_LEFT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[123] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillLeft" ),
     // Slot Nr. 122 : 26228
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_TAB,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[127] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       43/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillTable" ),
     // Slot Nr. 123 : 26229
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_SERIES,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[124] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       44/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillSeries" ),
     // Slot Nr. 124 : 26230
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_SINGLE_EDIT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[125] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FillSingleEdit" ),
     // Slot Nr. 125 : 26236
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DEL_ROWS,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[126] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteRows" ),
     // Slot Nr. 126 : 26237
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DEL_COLS,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[132] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteColumns" ),
     // Slot Nr. 127 : 26261
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_ROWBRK,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[128] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRowBreak" ),
     // Slot Nr. 128 : 26262
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_COLBRK,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[129] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumnBreak" ),
     // Slot Nr. 129 : 26263
     SFX_NEW_SLOT_ARG( ScCellShell,FID_DEL_ROWBRK,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[130] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteRowbreak" ),
     // Slot Nr. 130 : 26264
     SFX_NEW_SLOT_ARG( ScCellShell,FID_DEL_COLBRK,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[131] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteColumnbreak" ),
     // Slot Nr. 131 : 26265
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_CELL_CONTENTS,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[135] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       50/*Offset*/, 6/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertContents" ),
     // Slot Nr. 132 : 26266
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_CELL,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[133] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       56/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertCell" ),
     // Slot Nr. 133 : 26267
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_ROW,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[134] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       57/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRows" ),
     // Slot Nr. 134 : 26268
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_COLUMN,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[140] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       57/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumns" ),
     // Slot Nr. 135 : 26270
     SFX_NEW_SLOT_ARG( ScCellShell,SID_INS_FUNCTION,SfxGroupId::View,
                       &aScCellShellSlots_Impl[136] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       57/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertFunction" ),
     // Slot Nr. 136 : 26271
     SFX_NEW_SLOT_ARG( ScCellShell,FID_DEFINE_NAME,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[137] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       57/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineName" ),
     // Slot Nr. 137 : 26272
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INSERT_NAME,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[138] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertName" ),
     // Slot Nr. 138 : 26273
     SFX_NEW_SLOT_ARG( ScCellShell,FID_USE_NAME,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[139] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CreateNames" ),
     // Slot Nr. 139 : 26276
     SFX_NEW_SLOT_ARG( ScCellShell,FID_ADD_NAME,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[146] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AddName" ),
     // Slot Nr. 140 : 26278
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_CELLSDOWN,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[141] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertCellsDown" ),
     // Slot Nr. 141 : 26279
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_CELLSRIGHT,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[142] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertCellsRight" ),
     // Slot Nr. 142 : 26280
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_ROWS_AFTER,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[143] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRowsAfter" ),
     // Slot Nr. 143 : 26281
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_COLUMNS_AFTER,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[144] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumnsAfter" ),
     // Slot Nr. 144 : 26282
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_ROWS_BEFORE,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[145] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertRowsBefore" ),
     // Slot Nr. 145 : 26283
     SFX_NEW_SLOT_ARG( ScCellShell,FID_INS_COLUMNS_BEFORE,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[147] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertColumnsBefore" ),
     // Slot Nr. 146 : 26284
     SFX_NEW_SLOT_ARG( ScCellShell,FID_DEFINE_CURRENT_NAME,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[148] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       60/*Offset*/, 3/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineCurrentName" ),
     // Slot Nr. 147 : 26285
     SFX_NEW_SLOT_ARG( ScCellShell,FID_CELL_FORMAT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[227] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERITEM|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       63/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"FormatCellDialog" ),
     // Slot Nr. 148 : 26286
     SFX_NEW_SLOT_ARG( ScCellShell,FID_ROW_HEIGHT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[149] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       63/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RowHeight" ),
     // Slot Nr. 149 : 26287
     SFX_NEW_SLOT_ARG( ScCellShell,FID_ROW_OPT_HEIGHT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[150] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       65/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetOptimalRowHeight" ),
     // Slot Nr. 150 : 26288
     SFX_NEW_SLOT_ARG( ScCellShell,FID_ROW_HIDE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[151] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       66/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideRow" ),
     // Slot Nr. 151 : 26289
     SFX_NEW_SLOT_ARG( ScCellShell,FID_ROW_SHOW,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[152] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       66/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowRow" ),
     // Slot Nr. 152 : 26290
     SFX_NEW_SLOT_ARG( ScCellShell,FID_COL_WIDTH,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[153] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       66/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ColumnWidth" ),
     // Slot Nr. 153 : 26291
     SFX_NEW_SLOT_ARG( ScCellShell,FID_COL_OPT_WIDTH,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[154] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       68/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetOptimalColumnWidth" ),
     // Slot Nr. 154 : 26292
     SFX_NEW_SLOT_ARG( ScCellShell,FID_COL_HIDE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[155] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       69/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideColumn" ),
     // Slot Nr. 155 : 26293
     SFX_NEW_SLOT_ARG( ScCellShell,FID_COL_SHOW,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[156] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       69/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowColumn" ),
     // Slot Nr. 156 : 26300
     SFX_NEW_SLOT_ARG( ScCellShell,FID_MERGE_ON,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[157] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       69/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MergeCells" ),
     // Slot Nr. 157 : 26301
     SFX_NEW_SLOT_ARG( ScCellShell,FID_MERGE_OFF,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[158] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       70/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SplitCell" ),
     // Slot Nr. 158 : 26306
     SFX_NEW_SLOT_ARG( ScCellShell,FID_COL_OPT_DIRECT,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[159] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       70/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetOptimalColumnWidthDirect" ),
     // Slot Nr. 159 : 26319
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SCENARIOS,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[171] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       70/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ScenarioManager" ),
     // Slot Nr. 160 : 26327
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DEFINE_DBNAME,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[161] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       72/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineDBName" ),
     // Slot Nr. 161 : 26328
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECT_DB,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[162] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       72/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectDB" ),
     // Slot Nr. 162 : 26329
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SORT,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[163] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       73/*Offset*/, 14/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataSort" ),
     // Slot Nr. 163 : 26330
     SFX_NEW_SLOT_ARG( ScCellShell,SID_FILTER,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[164] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       87/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterStandardFilter" ),
     // Slot Nr. 164 : 26331
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SPECIAL_FILTER,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[165] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       87/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterSpecialFilter" ),
     // Slot Nr. 165 : 26332
     SFX_NEW_SLOT_ARG( ScCellShell,SID_AUTO_FILTER,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[166] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       87/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterAutoFilter" ),
     // Slot Nr. 166 : 26333
     SFX_NEW_SLOT_ARG( ScCellShell,SID_UNFILTER,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[167] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       87/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterRemoveFilter" ),
     // Slot Nr. 167 : 26335
     SFX_NEW_SLOT_ARG( ScCellShell,SID_MANAGE_XML_SOURCE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[168] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       87/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ManageXMLSource" ),
     // Slot Nr. 168 : 26340
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SUBTOTALS,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[169] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataSubTotals" ),
     // Slot Nr. 169 : 26343
     SFX_NEW_SLOT_ARG( ScCellShell,SID_REIMPORT_DATA,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[170] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataReImport" ),
     // Slot Nr. 170 : 26344
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PIVOT_TABLE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[172] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataPilotExec" ),
     // Slot Nr. 171 : 26345
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TABOP,SfxGroupId::View,
                       &aScCellShellSlots_Impl[173] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TableOperation" ),
     // Slot Nr. 172 : 26346
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TEXT_TO_COLUMNS,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[174] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"TextToColumns" ),
     // Slot Nr. 173 : 26347
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CONSOLIDATE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[177] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ConsolidateExec" ),
     // Slot Nr. 174 : 26348
     SFX_NEW_SLOT_ARG( ScCellShell,SID_AUTOFILTER_HIDE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[175] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataFilterHideAutoFilter" ),
     // Slot Nr. 175 : 26350
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SORT_DESCENDING,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[176] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SortDescending" ),
     // Slot Nr. 176 : 26351
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SORT_ASCENDING,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[250] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SortAscending" ),
     // Slot Nr. 177 : 26352
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OUTLINE_HIDE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[178] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"HideDetail" ),
     // Slot Nr. 178 : 26353
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OUTLINE_SHOW,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[179] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowDetail" ),
     // Slot Nr. 179 : 26354
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OUTLINE_MAKE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[180] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       88/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Group" ),
     // Slot Nr. 180 : 26355
     SFX_NEW_SLOT_ARG( ScCellShell,SID_OUTLINE_REMOVE,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[181] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       89/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Ungroup" ),
     // Slot Nr. 181 : 26389
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECT_SCENARIO,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[183] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       90/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectScenario" ),
     // Slot Nr. 182 : 26400
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PROPERTY_PANEL_CELLTEXT_DLG,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[228] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CellTextDlg" ),
     // Slot Nr. 183 : 26467
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_ADD_PRED,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[184] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowPrecedents" ),
     // Slot Nr. 184 : 26468
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_DEL_PRED,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[185] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClearArrowPrecedents" ),
     // Slot Nr. 185 : 26469
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_ADD_SUCC,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[186] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowDependents" ),
     // Slot Nr. 186 : 26470
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_DEL_SUCC,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[187] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClearArrowDependents" ),
     // Slot Nr. 187 : 26471
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_ADD_ERR,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[188] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowErrors" ),
     // Slot Nr. 188 : 26473
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_FILLMODE,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[189] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::NORECORD|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AuditingFillMode" ),
     // Slot Nr. 189 : 26480
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_INVALID,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[190] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ShowInvalid" ),
     // Slot Nr. 190 : 26481
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_REFRESH,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[191] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"RefreshArrows" ),
     // Slot Nr. 191 : 26483
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_MARK_PRED,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[192] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MarkPrecedents" ),
     // Slot Nr. 192 : 26484
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DETECTIVE_MARK_SUCC,SfxGroupId::Options,
                       &aScCellShellSlots_Impl[193] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"MarkDependents" ),
     // Slot Nr. 193 : 26488
     SFX_NEW_SLOT_ARG( ScCellShell,SID_INSERT_MATRIX,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[194] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       91/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"InsertMatrix" ),
     // Slot Nr. 194 : 26489
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DELETE_NOTE,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[229] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::FASTCALL|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       92/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DeleteNote" ),
     // Slot Nr. 195 : 26521
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORDOWN_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[196] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       93/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDownSel" ),
     // Slot Nr. 196 : 26522
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORUP_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[197] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       94/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUpSel" ),
     // Slot Nr. 197 : 26523
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORLEFT_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[198] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       95/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeftSel" ),
     // Slot Nr. 198 : 26524
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORRIGHT_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[199] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       96/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRightSel" ),
     // Slot Nr. 199 : 26525
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGEDOWN_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[200] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       97/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDownBlockSel" ),
     // Slot Nr. 200 : 26526
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGEUP_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[201] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       98/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUpBlockSel" ),
     // Slot Nr. 201 : 26527
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGELEFT_,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[202] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       99/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeftBlock" ),
     // Slot Nr. 202 : 26528
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGELEFT_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[203] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       101/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeftBlockSel" ),
     // Slot Nr. 203 : 26529
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGERIGHT_,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[204] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       102/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRightBlock" ),
     // Slot Nr. 204 : 26530
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORPAGERIGHT_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[205] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       104/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRightBlockSel" ),
     // Slot Nr. 205 : 26531
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORTOPOFFILE_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[206] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePageSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToStartSel" ),
     // Slot Nr. 206 : 26532
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORENDOFFILE_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[207] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePageSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToEndOfDataSel" ),
     // Slot Nr. 207 : 26533
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORHOME_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[208] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePageSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToStartOfRowSel" ),
     // Slot Nr. 208 : 26534
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSOREND_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[209] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecutePageSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToEndOfRowSel" ),
     // Slot Nr. 209 : 26535
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKUP,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[210] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       105/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUpToStartOfData" ),
     // Slot Nr. 210 : 26536
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKDOWN,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[211] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       107/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDownToEndOfData" ),
     // Slot Nr. 211 : 26537
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKLEFT,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[212] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       109/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeftToStartOfData" ),
     // Slot Nr. 212 : 26538
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKRIGHT,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[213] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursor),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       111/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRightToEndOfData" ),
     // Slot Nr. 213 : 26539
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKUP_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[214] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       113/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoUpToStartOfDataSel" ),
     // Slot Nr. 214 : 26540
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKDOWN_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[215] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       114/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoDownToEndOfDataSel" ),
     // Slot Nr. 215 : 26541
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKLEFT_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[216] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       115/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoLeftToStartOfDataSel" ),
     // Slot Nr. 216 : 26542
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORBLKRIGHT_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[217] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteCursorSel),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       116/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoRightToEndOfDataSel" ),
     // Slot Nr. 217 : 26543
     SFX_NEW_SLOT_ARG( ScCellShell,SID_NEXT_TABLE,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[218] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       117/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToNextTable" ),
     // Slot Nr. 218 : 26544
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PREV_TABLE,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[219] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       117/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToPrevTable" ),
     // Slot Nr. 219 : 26545
     SFX_NEW_SLOT_ARG( ScCellShell,SID_NEXT_UNPROTECT,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[220] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       117/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToNextUnprotected" ),
     // Slot Nr. 220 : 26546
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PREV_UNPROTECT,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[221] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       117/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToPreviousUnprotected" ),
     // Slot Nr. 221 : 26547
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECT_COL,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[222] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       117/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectColumn" ),
     // Slot Nr. 222 : 26548
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECT_ROW,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[223] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       119/*Offset*/, 2/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectRow" ),
     // Slot Nr. 223 : 26549
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECT_NONE,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[224] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Deselect" ),
     // Slot Nr. 224 : 26550
     SFX_NEW_SLOT_ARG( ScCellShell,SID_ALIGNCURSOR,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[225] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GoToCurrentCell" ),
     // Slot Nr. 225 : 26551
     SFX_NEW_SLOT_ARG( ScCellShell,SID_MARKDATAAREA,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[226] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectData" ),
     // Slot Nr. 226 : 26552
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SETINPUTMODE,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[231] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SetInputMode" ),
     // Slot Nr. 227 : 26553
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DELETE_CONTENTS,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[247] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ClearContents" ),
     // Slot Nr. 228 : 26554
     SFX_NEW_SLOT_ARG( ScCellShell,SID_MARKAREA,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[230] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectArea" ),
     // Slot Nr. 229 : 26556
     SFX_NEW_SLOT_ARG( ScCellShell,FID_FILL_AUTO,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[236] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       121/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AutoFill" ),
     // Slot Nr. 230 : 26557
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CANCEL,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[256] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::ASYNCHRON|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Cancel" ),
     // Slot Nr. 231 : 26558
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORENTERUP,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[232] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToPreviousCell" ),
     // Slot Nr. 232 : 26559
     SFX_NEW_SLOT_ARG( ScCellShell,SID_CURSORENTERDOWN,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[233] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToNextCell" ),
     // Slot Nr. 233 : 26560
     SFX_NEW_SLOT_ARG( ScCellShell,SID_MARKARRAYFORMULA,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[234] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectArrayFormula" ),
     // Slot Nr. 234 : 26561
     SFX_NEW_SLOT_ARG( ScCellShell,SID_NEXT_TABLE_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[235] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToNextTableSel" ),
     // Slot Nr. 235 : 26562
     SFX_NEW_SLOT_ARG( ScCellShell,SID_PREV_TABLE_SEL,SfxGroupId::Navigator,
                       &aScCellShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteMove),SFX_STUB_PTR(ScCellShell,GetStateCursor),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"JumpToPrevTableSel" ),
     // Slot Nr. 236 : 26581
     SFX_NEW_SLOT_ARG( ScCellShell,FID_MERGE_TOGGLE,SfxGroupId::Format,
                       &aScCellShellSlots_Impl[237] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       122/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleMergeCells" ),
     // Slot Nr. 237 : 26591
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_ROW,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[238] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt32Item,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"Row" ),
     // Slot Nr. 238 : 26592
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_COL,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[239] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"Column" ),
     // Slot Nr. 239 : 26593
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_TABLE,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[240] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"Table" ),
     // Slot Nr. 240 : 26595
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_FORMULA,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[241] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"Formula" ),
     // Slot Nr. 241 : 26597
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_TEXTVALUE,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[242] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"TextValue" ),
     // Slot Nr. 242 : 26609
     SFX_NEW_SLOT_ARG( ScCellShell,SID_TOGGLE_REL,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[243] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ToggleRelative" ),
     // Slot Nr. 243 : 26610
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DATA_SELECT,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[244] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,Execute),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataSelect" ),
     // Slot Nr. 244 : 26618
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_ADDRESS,SfxGroupId::NONE,
                       &aScCellShellSlots_Impl[245] /*Offset Next*/,
                       SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"Address" ),
     // Slot Nr. 245 : 26622
     SFX_NEW_SLOT_ARG( ScCellShell,FID_CURRENTVALIDATION,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[246] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CurrentValidation" ),
     // Slot Nr. 246 : 26623
     SFX_NEW_SLOT_ARG( ScCellShell,SID_RANGE_NOTETEXT,SfxGroupId::Intern,
                       &aScCellShellSlots_Impl[248] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxStringItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"NoteText" ),
     // Slot Nr. 247 : 26625
     SFX_NEW_SLOT_ARG( ScCellShell,FID_VALIDATION,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetBlockState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Validation" ),
     // Slot Nr. 248 : 26629
     SFX_NEW_SLOT_ARG( ScCellShell,SID_DEFINE_COLROWNAMERANGES,SfxGroupId::Insert,
                       &aScCellShellSlots_Impl[249] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DefineLabelRange" ),
     // Slot Nr. 249 : 26630
     SFX_NEW_SLOT_ARG( ScCellShell,FID_NOTE_VISIBLE,SfxGroupId::View,
                       &aScCellShellSlots_Impl[251] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"NoteVisible" ),
     // Slot Nr. 250 : 26643
     SFX_NEW_SLOT_ARG( ScCellShell,SID_REFRESH_DBAREA,SfxGroupId::Data,
                       &aScCellShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteDB),SFX_STUB_PTR(ScCellShell,GetDBState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"DataAreaRefresh" ),
     // Slot Nr. 251 : 26671
     SFX_NEW_SLOT_ARG( ScCellShell,FID_SHOW_NOTE,SfxGroupId::View,
                       &aScCellShellSlots_Impl[252] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"ShowNote" ),
     // Slot Nr. 252 : 26672
     SFX_NEW_SLOT_ARG( ScCellShell,FID_HIDE_NOTE,SfxGroupId::View,
                       &aScCellShellSlots_Impl[253] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"HideNote" ),
     // Slot Nr. 253 : 26673
     SFX_NEW_SLOT_ARG( ScCellShell,FID_SHOW_ALL_NOTES,SfxGroupId::View,
                       &aScCellShellSlots_Impl[254] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"ShowAllNotes" ),
     // Slot Nr. 254 : 26674
     SFX_NEW_SLOT_ARG( ScCellShell,FID_HIDE_ALL_NOTES,SfxGroupId::View,
                       &aScCellShellSlots_Impl[255] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"HideAllNotes" ),
     // Slot Nr. 255 : 26675
     SFX_NEW_SLOT_ARG( ScCellShell,FID_DELETE_ALL_NOTES,SfxGroupId::View,
                       &aScCellShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR(ScCellShell,GetState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0, SfxSlotMode::NONE,"DeleteAllNotes" ),
     // Slot Nr. 256 : 26677
     SFX_NEW_SLOT_ARG( ScCellShell,SID_SELECT_UNPROTECTED_CELLS,SfxGroupId::Edit,
                       &aScCellShellSlots_Impl[33] /*Offset Next*/,
                       SFX_STUB_PTR(ScCellShell,ExecuteEdit),SFX_STUB_PTR_STATE_NONE,
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       123/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SelectUnprotectedCells" )
    };
#endif

/************************************************************/
#ifdef ShellClass_ScOleObjectShell
#undef ShellClass
#undef ShellClass_ScOleObjectShell
#define ShellClass ScOleObjectShell
static SfxFormalArgument aScOleObjectShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };


static SfxSlot aScOleObjectShellSlots_Impl[] =
    {
     SFX_SLOT_ARG(ScOleObjectShell, 0, SfxGroupId::NONE, SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,SfxSlotMode::NONE, SfxVoidItem, 0, 0, "", SfxSlotMode::NONE )

    };
#endif

/************************************************************/
#ifdef ShellClass_ChartSelection
#undef ShellClass
#undef ShellClass_ChartSelection
#define ShellClass ChartSelection
#endif

/************************************************************/
#ifdef ShellClass_ScChartShell
#undef ShellClass
#undef ShellClass_ScChartShell
#define ShellClass ScChartShell
static SfxFormalArgument aScChartShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };

SFX_EXEC_STUB(ScChartShell,ExecuteExportAsGraphic)
SFX_STATE_STUB(ScChartShell,GetExportAsGraphicState)

static SfxSlot aScChartShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 26573
     SFX_NEW_SLOT_ARG( ScChartShell,SID_EXPORT_AS_GRAPHIC,SfxGroupId::Chart,
                       &aScChartShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScChartShell,ExecuteExportAsGraphic),SFX_STUB_PTR(ScChartShell,GetExportAsGraphicState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ExportAsGraphic" )
    };
#endif

/************************************************************/
#ifdef ShellClass_GraphSelection
#undef ShellClass
#undef ShellClass_GraphSelection
#define ShellClass GraphSelection
#endif

/************************************************************/
#ifdef ShellClass_ScGraphicShell
#undef ShellClass
#undef ShellClass_ScGraphicShell
#define ShellClass ScGraphicShell
static SfxFormalArgument aScGraphicShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };

SFX_EXEC_STUB(ScGraphicShell,ExecuteFilter)
SFX_STATE_STUB(ScGraphicShell,GetFilterState)
SFX_EXEC_STUB(ScGraphicShell,Execute)
SFX_STATE_STUB(ScGraphicShell,GetAttrState)
SFX_EXEC_STUB(ScGraphicShell,ExecuteExternalEdit)
SFX_STATE_STUB(ScGraphicShell,GetExternalEditState)
SFX_EXEC_STUB(ScGraphicShell,ExecuteChangePicture)
SFX_STATE_STUB(ScGraphicShell,GetChangePictureState)
SFX_EXEC_STUB(ScGraphicShell,ExecuteSaveGraphic)
SFX_STATE_STUB(ScGraphicShell,GetSaveGraphicState)
SFX_EXEC_STUB(ScGraphicShell,ExecuteCompressGraphic)
SFX_STATE_STUB(ScGraphicShell,GetCompressGraphicState)
SFX_EXEC_STUB(ScGraphicShell,ExecuteCropGraphic)
SFX_STATE_STUB(ScGraphicShell,GetCropGraphicState)

static SfxSlot aScGraphicShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 10469
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[1] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GraphicFilterToolbox" ),
     // Slot Nr. 1 : 10470
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_INVERT,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[2] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterInvert" ),
     // Slot Nr. 2 : 10471
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_SMOOTH,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[3] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSmooth" ),
     // Slot Nr. 3 : 10472
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_SHARPEN,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[4] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSharpen" ),
     // Slot Nr. 4 : 10473
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_REMOVENOISE,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[5] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterRemoveNoise" ),
     // Slot Nr. 5 : 10474
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_SOBEL,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[6] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSobel" ),
     // Slot Nr. 6 : 10475
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_MOSAIC,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[7] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterMosaic" ),
     // Slot Nr. 7 : 10476
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_EMBOSS,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[8] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterRelief" ),
     // Slot Nr. 8 : 10477
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_POSTER,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[9] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterPoster" ),
     // Slot Nr. 9 : 10478
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_POPART,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[10] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterPopart" ),
     // Slot Nr. 10 : 10479
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_SEPIA,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[11] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSepia" ),
     // Slot Nr. 11 : 10480
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_GRFFILTER_SOLARIZE,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteFilter),SFX_STUB_PTR(ScGraphicShell,GetFilterState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GraphicFilterSolarize" ),
     // Slot Nr. 12 : 10863
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_LUMINANCE,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[13] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafLuminance" ),
     // Slot Nr. 13 : 10864
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_CONTRAST,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[14] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafContrast" ),
     // Slot Nr. 14 : 10865
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_RED,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[15] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafRed" ),
     // Slot Nr. 15 : 10866
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_GREEN,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[16] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafGreen" ),
     // Slot Nr. 16 : 10867
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_BLUE,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[17] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafBlue" ),
     // Slot Nr. 17 : 10868
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_GAMMA,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[18] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt32Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafGamma" ),
     // Slot Nr. 18 : 10869
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_TRANSPARENCE,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[19] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafTransparence" ),
     // Slot Nr. 19 : 10871
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_MODE,SfxGroupId::Format,
                       &aScGraphicShellSlots_Impl[20] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxUInt16Item,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"GrafMode" ),
     // Slot Nr. 20 : 10883
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_ATTR_GRAF_CROP,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[21] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"GrafAttrCrop" ),
     // Slot Nr. 21 : 11044
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_COLOR_SETTINGS,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[12] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,Execute),SFX_STUB_PTR(ScGraphicShell,GetAttrState),
                       SfxSlotMode::TOGGLE|SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"ColorSettings" ),
     // Slot Nr. 22 : 11113
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_EXTERNAL_EDIT,SfxGroupId::Graphic,
                       &aScGraphicShellSlots_Impl[22] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteExternalEdit),SFX_STUB_PTR(ScGraphicShell,GetExternalEditState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0, SfxSlotMode::NONE,"ExternalEdit" ),
     // Slot Nr. 23 : 11115
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_CHANGE_PICTURE,SfxGroupId::Graphic,
                       &aScGraphicShellSlots_Impl[23] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteChangePicture),SFX_STUB_PTR(ScGraphicShell,GetChangePictureState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"ChangePicture" ),
     // Slot Nr. 24 : 11116
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_SAVE_GRAPHIC,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[24] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteSaveGraphic),SFX_STUB_PTR(ScGraphicShell,GetSaveGraphicState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"SaveGraphic" ),
     // Slot Nr. 25 : 11117
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_COMPRESS_GRAPHIC,SfxGroupId::Modify,
                       &aScGraphicShellSlots_Impl[25] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteCompressGraphic),SFX_STUB_PTR(ScGraphicShell,GetCompressGraphicState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxVoidItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"CompressGraphic" ),
     // Slot Nr. 26 : 11147
     SFX_NEW_SLOT_ARG( ScGraphicShell,SID_OBJECT_CROP,SfxGroupId::Document,
                       &aScGraphicShellSlots_Impl[26] /*Offset Next*/,
                       SFX_STUB_PTR(ScGraphicShell,ExecuteCropGraphic),SFX_STUB_PTR(ScGraphicShell,GetCropGraphicState),
                       SfxSlotMode::AUTOUPDATE|SfxSlotMode::RECORDPERSET|SfxSlotMode::MENUCONFIG|SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       SfxBoolItem,
                       0/*Offset*/, 0/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"Crop" )
    };
#endif

/************************************************************/
#ifdef ShellClass_ScPageBreakShell
#undef ShellClass
#undef ShellClass_ScPageBreakShell
#define ShellClass ScPageBreakShell
static SfxFormalArgument aScPageBreakShellArgs_Impl[] =
    {
     { (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }

    };


static SfxSlot aScPageBreakShellSlots_Impl[] =
    {
     SFX_SLOT_ARG(ScPageBreakShell, 0, SfxGroupId::NONE, SFX_STUB_PTR_EXEC_NONE,SFX_STUB_PTR_STATE_NONE,SfxSlotMode::NONE, SfxVoidItem, 0, 0, "", SfxSlotMode::NONE )

    };
#endif

/************************************************************/
#ifdef ShellClass_MediaSelection
#undef ShellClass
#undef ShellClass_MediaSelection
#define ShellClass MediaSelection
#endif

/************************************************************/
#ifdef ShellClass_ScMediaShell
#undef ShellClass
#undef ShellClass_ScMediaShell
#define ShellClass ScMediaShell
static SfxFormalArgument aScMediaShellArgs_Impl[] =
    {
     { (const SfxType*) &aavmedia_MediaItem_Impl, "AVMediaToolBox", SID_AVMEDIA_TOOLBOX }
    };

SFX_EXEC_STUB(ScMediaShell,ExecuteMedia)
SFX_STATE_STUB(ScMediaShell,GetMediaState)

static SfxSlot aScMediaShellSlots_Impl[] =
    {
     // Slot Nr. 0 : 6693
     SFX_NEW_SLOT_ARG( ScMediaShell,SID_AVMEDIA_TOOLBOX,SfxGroupId::Application,
                       &aScMediaShellSlots_Impl[0] /*Offset Next*/,
                       SFX_STUB_PTR(ScMediaShell,ExecuteMedia),SFX_STUB_PTR(ScMediaShell,GetMediaState),
                       SfxSlotMode::RECORDPERSET|SfxSlotMode::READONLYDOC|SfxSlotMode::NONE,
                       SfxDisableFlags::NONE,
                       avmedia_MediaItem,
                       0/*Offset*/, 1/*Count*/, SfxSlotMode::METHOD|SfxSlotMode::NONE,"AVMediaToolBox" )
    };
#endif
