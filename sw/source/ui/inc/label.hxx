/*************************************************************************
 *
 *  $RCSfile: label.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

class SwLabRec;
class SwLabRecs;
class SwLabItem;
class SwLabPrtPage;
class SwNewDBMgr;
class Printer;

class SwLabDlg : public SfxTabDialog
{
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
    void          _ReplaceGroup( const String &rMake, SwLabItem *pItem );

    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );
public:

     SwLabDlg( Window* pParent, const SfxItemSet& rSet,
                 SwNewDBMgr* pNewDBMgr, BOOL bLabel);
    ~SwLabDlg();

    void MakeConfigItem(SwLabItem& rItem) const;

    SwLabRec*   GetRecord(const String &rRecName, BOOL bCont);
    void        GetLabItem(SwLabItem &rItem);

          SwLabRecs &Recs()           { return *pRecs;   }
    const SwLabRecs &Recs()     const { return *pRecs;   }

          SvUShorts  &TypeIds()       { return aTypeIds; }
    const SvUShorts  &TypeIds() const { return aTypeIds; }

          SvStringsDtor  &Makes()         { return aMakes;   }
    const SvStringsDtor  &Makes()   const { return aMakes;   }

    Printer *GetPrt();
    inline void ReplaceGroup( const String &rMake, SwLabItem *pItem );
    static void UpdateFieldInformation(::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
                                                                                const SwLabItem& rItem);
    const String& GetBusinessCardStr() const {return sBusinessCardDlg;}

};

inline void SwLabDlg::ReplaceGroup( const String &rMake, SwLabItem *pItem )
{
    if ( rMake != aLstGroup )
        _ReplaceGroup( rMake, pItem );
}

#endif

