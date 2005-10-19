/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: label.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 08:31:09 $
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
#ifndef _LABEL_HXX
#define _LABEL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _LABELCFG_HXX
#include <labelcfg.hxx>
#endif
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
    BOOL            m_bLabel;
    void          _ReplaceGroup( const String &rMake );

    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );
public:

     SwLabDlg( Window* pParent, const SfxItemSet& rSet,
                 SwNewDBMgr* pNewDBMgr, BOOL bLabel);
    ~SwLabDlg();

    SwLabRec*   GetRecord(const String &rRecName, BOOL bCont);
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

