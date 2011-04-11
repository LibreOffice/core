/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _LABEL_HXX
#define _LABEL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <labelcfg.hxx>
class SwLabRec;
class SwLabRecs;
class SwLabItem;
class SwLabPrtPage;
class SwNewDBMgr;
class Printer;

class SwLabDlg : public SfxTabDialog
{
    SwLabelConfig   aLabelsCfg;
    SwNewDBMgr*     pNewDBMgr;
    SwLabPrtPage*   pPrtPage;

    SvUShorts       aTypeIds;
    SvStringsDtor   aMakes;

    SwLabRecs*      pRecs;
    String          aLstGroup;
    String          sBusinessCardDlg;
    String          sFormat;
    String          sMedium;
    sal_Bool            m_bLabel;
    void          _ReplaceGroup( const String &rMake );

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
public:

     SwLabDlg( Window* pParent, const SfxItemSet& rSet,
                 SwNewDBMgr* pNewDBMgr, sal_Bool bLabel);
    ~SwLabDlg();

    SwLabRec*   GetRecord(const String &rRecName, sal_Bool bCont);
    void        GetLabItem(SwLabItem &rItem);

          SwLabRecs &Recs()           { return *pRecs;   }
    const SwLabRecs &Recs()     const { return *pRecs;   }

          SvUShorts  &TypeIds()       { return aTypeIds; }
    const SvUShorts  &TypeIds() const { return aTypeIds; }

          SvStringsDtor  &Makes()         { return aMakes;   }
    const SvStringsDtor  &Makes()   const { return aMakes;   }

    Printer *GetPrt();
    inline void ReplaceGroup( const String &rMake );
    void UpdateGroup( const String &rMake ) {_ReplaceGroup( rMake );}
    static void UpdateFieldInformation(::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
                                                                                const SwLabItem& rItem);
    const String& GetBusinessCardStr() const {return sBusinessCardDlg;}

    SwLabelConfig& GetLabelsConfig() {return aLabelsCfg;}

};

inline void SwLabDlg::ReplaceGroup( const String &rMake )
{
    if ( rMake != aLstGroup )
        _ReplaceGroup( rMake );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
