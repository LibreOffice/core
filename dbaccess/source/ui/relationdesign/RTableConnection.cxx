/*************************************************************************
 *
 *  $RCSfile: RTableConnection.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:06:52 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_RTABLECONNECTION_HXX
#include "RTableConnection.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#include "RelationTableView.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace dbaui;
//========================================================================
// class ORelationTableConnection
//========================================================================
DBG_NAME(ORelationTableConnection);
//------------------------------------------------------------------------
ORelationTableConnection::ORelationTableConnection( ORelationTableView* pContainer,
                                                   ORelationTableConnectionData* pTabConnData )
    :OTableConnection( pContainer, pTabConnData )
{
    DBG_CTOR(ORelationTableConnection,NULL);
}

//------------------------------------------------------------------------
ORelationTableConnection::ORelationTableConnection( const ORelationTableConnection& rConn )
    : OTableConnection( rConn )
{
    DBG_CTOR(ORelationTableConnection,NULL);
    // keine eigenen Members, also reicht die Basisklassenfunktionalitaet
}

//------------------------------------------------------------------------
ORelationTableConnection::~ORelationTableConnection()
{
    DBG_DTOR(ORelationTableConnection,NULL);
}

//------------------------------------------------------------------------
OConnectionLine* ORelationTableConnection::CreateConnLine( const OConnectionLine& rConnLine )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    return new OConnectionLine( rConnLine );
}

//------------------------------------------------------------------------
ORelationTableConnection& ORelationTableConnection::operator=( const ORelationTableConnection& rConn )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    // nicht dass es was aendern wuerde, da die Basisklasse das auch testet und ich keine eigenen Members zu kopieren habe
    if (&rConn == this)
        return *this;

    OTableConnection::operator=( rConn );
    return *this;
}


//------------------------------------------------------------------------
void ORelationTableConnection::Draw( const Rectangle& rRect )
{
    DBG_CHKTHIS(ORelationTableConnection,NULL);
    OTableConnection::Draw( rRect );
    ORelationTableConnectionData* pData;

    if ((pData = (ORelationTableConnectionData*) GetData()) &&
                (pData->GetCardinality() == CARDINAL_UNDEFINED) )
        return;

    //////////////////////////////////////////////////////////////////////
    // Linien nach oberster Linie durchsuchen
    Rectangle aBoundingRect;
    long nTop = GetBoundingRect().Bottom();
    long nTemp;

    const OConnectionLine* pTopLine = NULL;
    const ::std::vector<OConnectionLine*>* pConnLineList = GetConnLineList();
    ::std::vector<OConnectionLine*>::const_iterator aIter = pConnLineList->begin();
    for(;aIter != pConnLineList->end();++aIter)
    {
        if( (*aIter)->IsValid() )
        {
            aBoundingRect = (*aIter)->GetBoundingRect();
            nTemp = aBoundingRect.Top();
            if( nTemp<nTop )
            {
                nTop = nTemp;
                pTopLine = (*aIter);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Kardinalitaet antragen
    if( !pTopLine )
        return;

    Rectangle aSourcePos = pTopLine->GetSourceTextPos();
    Rectangle aDestPos = pTopLine->GetDestTextPos();

    String aSourceText;
    String aDestText;

    switch( ((ORelationTableConnectionData*)GetData())->GetCardinality() )
    {
    case CARDINAL_ONE_MANY:
        aSourceText = '1';
        aDestText = 'n';
        break;

    case CARDINAL_MANY_ONE:
        aSourceText = 'n';
        aDestText = '1';
        break;

    case CARDINAL_ONE_ONE:
        aSourceText = '1';
        aDestText = '1';
        break;
    }

    if (IsSelected())
        GetParent()->SetTextColor(Application::GetSettings().GetStyleSettings().GetHighlightColor());
    else
        GetParent()->SetTextColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor());


    GetParent()->DrawText( aSourcePos, aSourceText, TEXT_DRAW_CLIP | TEXT_DRAW_CENTER | TEXT_DRAW_BOTTOM);
    GetParent()->DrawText( aDestPos, aDestText, TEXT_DRAW_CLIP | TEXT_DRAW_CENTER | TEXT_DRAW_BOTTOM);
}
// -----------------------------------------------------------------------------



