/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_FUHHCONV_HXX
#define _SD_FUHHCONV_HXX

#ifndef _SD_FUPOOR_HXX
#include "fupoor.hxx"
#endif

namespace sd {

class Outliner;


class FuHangulHanjaConversion : public FuPoor
{
 public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    void StartConversion( sal_Int16 nSourceLanguage,  sal_Int16 nTargetLanguage,
                          const Font *pTargetFont, sal_Int32 nOptions, sal_Bool bIsInteractive );

    void StartChineseConversion();

    void ConvertStyles( sal_Int16 nTargetLanguage, const Font *pTargetFont );

    Outliner* GetOutliner() const { return pSdOutliner; }

 protected:
    ~FuHangulHanjaConversion();

    Outliner*   pSdOutliner;
    sal_Bool            bOwnOutliner;

private:
    FuHangulHanjaConversion (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq );

};

#endif // _SD_FUHHCONV_HXX


} // end of namespace sd
