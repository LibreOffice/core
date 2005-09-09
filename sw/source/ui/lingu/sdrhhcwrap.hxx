/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrhhcwrap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:27:04 $
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
#ifndef _SDRHHCWRAP_HXX_
#define _SDRHHCWRAP_HXX_

#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

class SwView;
class SdrTextObj;
class OutlinerView;
class SdrObjListIter;

//////////////////////////////////////////////////////////////////////

class SdrHHCWrapper : public SdrOutliner
{
    // modified version of SdrSpeller

    SwView*             pView;
    SdrTextObj*         pTextObj;
    OutlinerView*       pOutlView;
    SdrObjListIter*     pListIter;
    sal_Int32           nOptions;
    sal_uInt16          nDocIndex;
    LanguageType        nSourceLang;
    LanguageType        nTargetLang;
    const Font*         pTargetFont;
    sal_Bool            bIsInteractive;

public:
    SdrHHCWrapper( SwView* pVw,
                   LanguageType nSourceLanguage, LanguageType nTargetLanguage,
                   const Font* pTargetFnt,
                   sal_Int32 nConvOptions, sal_Bool bInteractive );

    virtual ~SdrHHCWrapper();

    virtual sal_Bool ConvertNextDocument();
    void    StartTextConversion();
};

//////////////////////////////////////////////////////////////////////

#endif

