#ifndef _LAYOUT_PRE_HXX
#define _LAYOUT_PRE_HXX

#if ENABLE_LAYOUT

//#if !LAYOUT
//#define LAYOUT 1
//#endif
//#define LAYOUT_NS layout

// TWBN, but find that SfxModalDialog and FixedLine are already defined, eg compiling
// dlgfact.cxx
//typedef layout::Dialog SfxModalDialog;
//typedef layout::FixedLine FixedLine;

#define Button layout::Button
#define CancelButton layout::CancelButton
#define FixedInfo layout::FixedInfo
#define FixedLine layout::FixedLine
#define FixedText layout::FixedText
#define HelpButton layout::HelpButton
#define MetricField layout::MetricField
#define OKButton layout::OKButton
#define RadioButton layout::RadioButton
#define SfxModalDialog layout::Dialog

/* FIXME: why are we defaulting to layout::Window?
   /home/janneke/vc/ooo-build/build/hack/sw/source/ui/dialog/wordcountdialog.cxx:87: error: no matching function for call to 'layout::Dialog::Dialog(Window*&, const char [14], const char [7])'
   ../../../../layout/inc/layout/layout.hxx:304: note: candidates are: layout::Dialog::Dialog(layout::Window*, const char*, const char*, sal_uInt32)
   ../../../../layout/inc/layout/layout.hxx:300: note:                 layout::Dialog::Dialog(const layout::Dialog&)
*/

#define Window ::Window

#undef SVX_RES
#define SVX_RES(x) #x
#undef SW_RES
#define SW_RES(x) #x

/* Hmm.  This hack makes zoom.cxx, wordcountdialog.cxx diffs smaller
 * but is not scalable. */
#ifdef _LAYOUT_POST_HXX

#ifdef _SVX_ZOOM_CXX
#undef SfxModalDialog
#define SfxModalDialog( pParent, SVX_RES_RID ) layout::Dialog( pParent, "zoom.xml", "dialog" )
#define _SVX_ZOOM_HRC
#endif /* _SVX_ZOOM_CXX */

#ifdef SW_WORDCOUNTDIALOG_HXX
#undef SfxModalDialog
#define SfxModalDialog( pParent, SW_RES_RID ) layout::Dialog( pParent, "wordcount.xml", "dialog" )
#define SW_WORDCOUNTDIALOG_HRC
#endif /* SW_WORDCOUNTDIALOG_HXX */

#endif /* _SVX_ZOOM_CXX */

#else /* !ENABLE_LAYOUT */

#define LAYOUT_PRE_POST

#endif /* !ENABLE_LAYOUT */

#endif /* _LAYOUT_PRE_HXX */
