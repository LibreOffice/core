/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_HTMLFMTFLT_HXX
#define INCLUDED_HTMLFMTFLT_HXX

#include <com/sun/star/uno/Sequence.hxx>

/* Transform plain HTML into the format expected by MS Office.
 */
com::sun::star::uno::Sequence<sal_Int8> TextHtmlToHTMLFormat(com::sun::star::uno::Sequence<sal_Int8>& aTextHtml);

/* Transform the MS Office HTML format into plain HTML.
 */
com::sun::star::uno::Sequence<sal_Int8> HTMLFormatToTextHtml(const com::sun::star::uno::Sequence<sal_Int8>& aHTMLFormat);

/* Detects whether the given byte sequence contains the MS Office Html format.

   @returns True if the MS Office Html format will be detected False otherwise.
 */
bool isHTMLFormat (const com::sun::star::uno::Sequence<sal_Int8>& aHtmlSequence);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
