/*************************************************************************
 *
 *  $RCSfile: movedfwdfrmsbyobjpos.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:11:04 $
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
#ifndef _MOVEDFWDFRMSBYOBJPOS_HXX
#include <movedfwdfrmsbyobjpos.hxx>
#endif

#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif

SwMovedFwdFrmsByObjPos::SwMovedFwdFrmsByObjPos()
{
}

SwMovedFwdFrmsByObjPos::~SwMovedFwdFrmsByObjPos()
{
    Clear();
}

void SwMovedFwdFrmsByObjPos::Insert( const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                     const sal_uInt32 _nToPageNum )
{
    sal_uInt32 nDummy;
    if ( !FrmMovedFwdByObjPos( _rMovedFwdFrmByObjPos, nDummy ) )
    {
        Entry* pNewEntry = new Entry;

        pNewEntry->mpTxtNode = _rMovedFwdFrmByObjPos.GetTxtNode();
        pNewEntry->mnToPageNum = _nToPageNum;

        maMovedFwdFrms.push_back( pNewEntry );
    }
}

bool SwMovedFwdFrmsByObjPos::FrmMovedFwdByObjPos( const SwTxtFrm& _rTxtFrm,
                                                  sal_uInt32& _ornToPageNum ) const
{
    bool bFrmMovedFwd( false );

    std::vector< Entry* >::const_iterator aIter = maMovedFwdFrms.begin();
    for ( ; aIter != maMovedFwdFrms.end(); ++aIter )
    {
        const Entry* pCurrEntry = *(aIter);
        if ( pCurrEntry->mpTxtNode == _rTxtFrm.GetTxtNode() )
        {
            bFrmMovedFwd = true;
            _ornToPageNum = pCurrEntry->mnToPageNum;
            break;
        }
    }

    return bFrmMovedFwd;
}

void SwMovedFwdFrmsByObjPos::Clear()
{
    while ( maMovedFwdFrms.size() )
    {
        delete maMovedFwdFrms.back();
        maMovedFwdFrms.pop_back();
    }
}
