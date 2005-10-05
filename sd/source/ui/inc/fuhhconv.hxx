/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuhhconv.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 13:12:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
 protected:
    Outliner*   pSdOutliner;
    BOOL            bOwnOutliner;

 public:
    TYPEINFO();

    FuHangulHanjaConversion (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq );
    ~FuHangulHanjaConversion();

    void StartConversion( INT16 nSourceLanguage,  INT16 nTargetLanguage,
                          const Font *pTargetFont, INT32 nOptions, BOOL bIsInteractive );

    void StartChineseConversion();

    void ConvertStyles( INT16 nTargetLanguage, const Font *pTargetFont );

    Outliner* GetOutliner() const { return pSdOutliner; }
};

#endif // _SD_FUHHCONV_HXX


} // end of namespace sd
