#ifndef _LAYOUT_POST_HXX
#define _LAYOUT_POST_HXX

#if ENABLE_LAYOUT

#undef Button
#undef CancelButton
#undef FixedInfo
#undef FixedLine
#undef FixedText
#undef HelpButton
#undef MetricField
#undef OKButton
#undef RadioButton
#undef SfxModalDialog

#undef Window

#undef SVX_RES
#define SVX_RES(i)        ResId(i,DIALOG_MGR())

#undef SW_RES
#define SW_RES(i)               ResId(i,SWDIALOG_MGR())

/* Allow re-inclusion for cxx file. */
#undef _LAYOUT_PRE_HXX

#endif /* ENABLE_LAYOUT */

#endif /* _LAYOUT_POST_HXX */
