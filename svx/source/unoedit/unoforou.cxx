/*************************************************************************
 *
 *  $RCSfile: unoforou.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cl $ $Date: 2001-08-22 14:30:53 $
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

#pragma hdrstop

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#include <svtools/itemset.hxx>
#include <editeng.hxx>
#include <outliner.hxx>

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _EEITEM_HXX //autogen
#include "eeitem.hxx"
#endif

#include "unoforou.hxx"
#include "unofored.hxx"

//------------------------------------------------------------------------

SvxOutlinerForwarder::SvxOutlinerForwarder( Outliner& rOutl ) :
    rOutliner( rOutl )
{
}

SvxOutlinerForwarder::~SvxOutlinerForwarder()
{
    //  der Outliner muss ggf. von aussen geloescht werden
}

USHORT SvxOutlinerForwarder::GetParagraphCount() const
{
    return (USHORT)rOutliner.GetParagraphCount();
}

USHORT SvxOutlinerForwarder::GetTextLen( USHORT nParagraph ) const
{
    return rOutliner.GetEditEngine().GetTextLen( nParagraph );
}

String SvxOutlinerForwarder::GetText( const ESelection& rSel ) const
{
    //! GetText(ESelection) sollte es wohl auch mal am Outliner geben
    //  solange den Hack fuer die EditEngine uebernehmen:
    EditEngine* pEditEngine = (EditEngine*)&rOutliner.GetEditEngine();
    return pEditEngine->GetText( rSel, LINEEND_LF );
}

SfxItemSet SvxOutlinerForwarder::GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib ) const
{
    //! gibt's das nicht am Outliner ???
    //! und warum ist GetAttribs an der EditEngine nicht const?
    EditEngine& rEditEngine = (EditEngine&)rOutliner.GetEditEngine();

    SfxItemSet aSet( rEditEngine.GetAttribs( rSel, bOnlyHardAttrib ) );

    SfxStyleSheet* pStyle = rEditEngine.GetStyleSheet( rSel.nStartPara );
    if( pStyle )
        aSet.SetParent( &(pStyle->GetItemSet() ) );

    return aSet;
}

SfxItemSet SvxOutlinerForwarder::GetParaAttribs( USHORT nPara ) const
{
    SfxItemSet aSet( rOutliner.GetParaAttribs( nPara ) );

    EditEngine& rEditEngine = (EditEngine&)rOutliner.GetEditEngine();

    SfxStyleSheet* pStyle = rEditEngine.GetStyleSheet( nPara );
    if( pStyle )
        aSet.SetParent( &(pStyle->GetItemSet() ) );

    return aSet;
}

void SvxOutlinerForwarder::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
{
    const SfxItemSet* pOldParent = rSet.GetParent();
    if( pOldParent )
        ((SfxItemSet*)&rSet)->SetParent( NULL );

    rOutliner.SetParaAttribs( nPara, rSet );

    if( pOldParent )
        ((SfxItemSet*)&rSet)->SetParent( pOldParent );
}

SfxItemPool* SvxOutlinerForwarder::GetPool() const
{
    return rOutliner.GetEmptyItemSet().GetPool();
}

void SvxOutlinerForwarder::GetPortions( USHORT nPara, SvUShorts& rList ) const
{
    ((EditEngine&)rOutliner.GetEditEngine()).GetPortions( nPara, rList );
}

void SvxOutlinerForwarder::QuickInsertText( const String& rText, const ESelection& rSel )
{
    if( rText.Len() == 0 )
    {
        rOutliner.QuickDelete( rSel );
    }
    else
    {
        rOutliner.QuickInsertText( rText, rSel );
    }
}

void SvxOutlinerForwarder::QuickInsertLineBreak( const ESelection& rSel )
{
    rOutliner.QuickInsertLineBreak( rSel );
}

void SvxOutlinerForwarder::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    rOutliner.QuickInsertField( rFld, rSel );
}

void SvxOutlinerForwarder::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    rOutliner.QuickSetAttribs( rSet, rSel );
}

XubString SvxOutlinerForwarder::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return rOutliner.CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

extern USHORT GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, USHORT nWhich );

USHORT SvxOutlinerForwarder::GetItemState( const ESelection& rSel, USHORT nWhich ) const
{
    return GetSvxEditEngineItemState( (EditEngine&)rOutliner.GetEditEngine(), rSel, nWhich );
}

USHORT SvxOutlinerForwarder::GetItemState( USHORT nPara, USHORT nWhich ) const
{
    const SfxItemSet& rSet = rOutliner.GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}

//------------------------------------------------------------------------



