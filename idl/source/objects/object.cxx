/*************************************************************************
 *
 *  $RCSfile: object.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#include <ctype.h>
#include <stdio.h>

#include <tools/debug.hxx>

#include <attrib.hxx>
#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>
#pragma hdrstop


/******************** class SvClassElement *******************************/
SV_IMPL_PERSIST1( SvClassElement, SvPersistBase );

/*************************************************************************
|*    SvClassElement::SvClassElement()
|*
|*    Beschreibung
*************************************************************************/
SvClassElement::SvClassElement()
{
};

/*************************************************************************
|*    SvClassElement::Load()
|*
|*    Beschreibung
*************************************************************************/
void SvClassElement::Load( SvPersistStream & rStm )
{
    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x08 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "wrong format" )
        return;
    }
    if( nMask & 0x01 ) rStm >> aAutomation;
    if( nMask & 0x02 ) rStm.ReadByteString( aPrefix );
    if( nMask & 0x04 )
    {
        SvMetaClass * p;
        rStm >> p;
        xClass = p;
    }
}

/*************************************************************************
|*    SvClassElement::Save()
|*
|*    Beschreibung
*************************************************************************/
void SvClassElement::Save( SvPersistStream & rStm )
{
    // Maske erstellen
    BYTE nMask = 0;
    if( aAutomation.IsSet() )       nMask |= 0x1;
    if( aPrefix.Len() )             nMask |= 0x2;
    if( xClass.Is() )               nMask |= 0x4;

    // Daten schreiben
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aAutomation;
    if( nMask & 0x02 ) rStm.WriteByteString( aPrefix );
    if( nMask & 0x04 ) rStm << xClass;
}

/****************** SvMetaClass ******************************************/
SV_IMPL_META_FACTORY1( SvMetaClass, SvMetaType );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaClass::GetAttributeList()
{
    if( !pAttribList )
    {
        pAttribList = new SvAttributeList();
    }
    return *pAttribList;
}
#endif
/*************************************************************************
|*    SvMetaClass::SvMetaClass()
|*
|*    Beschreibung
*************************************************************************/
SvMetaClass::SvMetaClass()
    : aAutomation( TRUE, FALSE )
{
}

/*************************************************************************
|*    SvMetaClass::Load()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );

    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "wrong format" )
        return;
    }
    if( nMask & 0x01 ) rStm >> aAttrList;
    if( nMask & 0x02 )
    {
        SvMetaClass * pSuper;
         rStm >> pSuper;
        aSuperClass = pSuper;
    }
    if( nMask & 0x04 ) rStm >> aClassList;
    if( nMask & 0x8 )
    {
        SvMetaClass * p;
        rStm >> p;
        xAutomationInterface = p;
    }
    if( nMask & 0x10 ) rStm >> aAutomation;
}

/*************************************************************************
|*    SvMetaClass::Save()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );

    // Maske erstellen
    BYTE nMask = 0;
    if( aAttrList.Count() )         nMask |= 0x1;
    if( aSuperClass.Is() )          nMask |= 0x2;
    if( aClassList.Count() )        nMask |= 0x4;
    if( xAutomationInterface.Is() ) nMask |= 0x8;
    if( aAutomation.IsSet() )       nMask |= 0x10;

    // Daten schreiben
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aAttrList;
    if( nMask & 0x02 ) rStm << aSuperClass;
    if( nMask & 0x04 ) rStm << aClassList;
    if( nMask & 0x08 ) rStm << xAutomationInterface;
    if( nMask & 0x10 ) rStm << aAutomation;
}

/*************************************************************************
|*    SvMetaClass::FillSbxObject()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaClass::FillSbxMemberObject( SvIdlDataBase & rBase,
                                        SbxObject * pObj,
                                        StringList & rSuperList,
                                        BOOL bVariable )
{
    // alle Attribute der Klasse schreiben
    ULONG n ;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );

        ByteString aMangleName = pAttr->GetMangleName( bVariable );
        ByteString * pS = SvIdlDataBase::FindName( aMangleName, rSuperList );

        if( !pS && pAttr->GetExport() )
        {
            // nicht doppelt
            if( bVariable && pAttr->IsVariable() )
            {
                rSuperList.Insert( new ByteString( aMangleName ), LIST_APPEND );
                 pAttr->FillSbxObject( rBase, pObj, bVariable );
            }
            else if( !bVariable && pAttr->IsMethod() )
            {
                rSuperList.Insert( new ByteString( aMangleName ), LIST_APPEND );
                pAttr->FillSbxObject( rBase, pObj, bVariable );
            }
        }
    }
    // alle Attribute der importierten Klassen schreiben
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        SvMetaClass * pClass = pEle->GetClass();
        pClass->FillSbxMemberObject( rBase, pObj, rSuperList, bVariable );
    }
    // alle Attribute der Superklassen schreiben
    if( aSuperClass.Is() )
        aSuperClass->FillSbxMemberObject( rBase, pObj, rSuperList, bVariable );
}
*/
/*************************************************************************
|*    SvMetaClass::FillSbxObject()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaClass::FillSbxObject( SvIdlDataBase & rBase, SbxObject * pObj )
{
    StringList aSuperList;
    FillSbxMemberObject( rBase, pObj, aSuperList, TRUE );
    FillSbxMemberObject( rBase, pObj, aSuperList, FALSE );

    ByteString * pStr = aSuperList.First();
    while( pStr )
    {
        delete pStr;
        pStr = aSuperList.Next();
    }
}
 */
#ifdef IDL_COMPILER
/*************************************************************************
|*    SvMetaClass::ReadAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaType::ReadAttributesSvIdl( rBase, rInStm );
    aAutomation.ReadSvIdl( SvHash_Automation(), rInStm );
}

/*************************************************************************
|*    SvMetaClass::WriteAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                 SvStream & rOutStm, USHORT nTab )
{
    SvMetaType::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    if( !aAutomation )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "//class SvMetaClass" << endl;
        if( !aAutomation )
        {
            WriteTab( rOutStm, nTab );
            aAutomation.WriteSvIdl( SvHash_Automation(), rOutStm );
            rOutStm << ';' << endl;
        }
    }
}

/*************************************************************************
|*    SvMetaClass::ReadContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::ReadContextSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    UINT32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_import() ) )
    {
        SvMetaClass * pClass = rBase.ReadKnownClass( rInStm );
        if( pClass )
        {
            SvClassElementRef xEle = new SvClassElement();
            xEle->SetClass( pClass );
            aClassList.Append( xEle );

            if( rInStm.Read( '[' ) )
            {
                pTok = rInStm.GetToken_Next();
                if( pTok->Is( SvHash_Automation() ) )
                {
                    if( rInStm.Read( ']' ) )
                    {
                        if( xAutomationInterface.Is() )
                        {
                            // Fehler setzen
                            rBase.SetError( "Automation allready set",
                                            rInStm.GetToken() );
                            rBase.WriteError( rInStm );
                        }
                        xAutomationInterface = pClass;
                        xEle->SetAutomation( TRUE );
                    }
                    else
                    {
                        // Fehler setzen
                        rBase.SetError( "missing ]", rInStm.GetToken() );
                        rBase.WriteError( rInStm );
                    }
                }
                else
                {
                    // Fehler setzen
                    rBase.SetError( "only attribute Automation allowed",
                                    rInStm.GetToken() );
                    rBase.WriteError( rInStm );
                }
            }
            pTok = rInStm.GetToken();
            if( pTok->IsString() )
            {
                xEle->SetPrefix( pTok->GetString() );
                rInStm.GetToken_Next();
            }
            return;
        }
        else
        {
            // Fehler setzen
            rBase.SetError( "unknown imported interface", rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    else
    {
        rInStm.Seek( nTokPos );
        SvMetaType * pType = rBase.ReadKnownType( rInStm );

        BOOL bOk = FALSE;
        SvMetaAttributeRef xAttr;
        if( !pType || pType->IsItem() )
        {
            xAttr = new SvMetaSlot( pType );
            if( xAttr->ReadSvIdl( rBase, rInStm ) )
                bOk = xAttr->Test( rBase, rInStm );
        }
        else
        {
            xAttr = new SvMetaAttribute( pType );
            if( xAttr->ReadSvIdl( rBase, rInStm ) )
                bOk = xAttr->Test( rBase, rInStm );
        }

        if( bOk )
            bOk = TestAttribute( rBase, rInStm, *xAttr );
        if( bOk )
        {
            if( !xAttr->GetSlotId().IsSet() )
            {
                SvNumberIdentifier aI;
                aI.SetValue( rBase.GetUniqueId() );
                xAttr->SetSlotId( aI );
            }
            aAttrList.Append( xAttr );
            return;
        }
    }
    rInStm.Seek( nTokPos );
}

/*************************************************************************
|*    SvMetaClass::WriteContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteContextSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab
)
{
    //SvMetaType::WriteContextSvIdl( rBase, rOutStm, nTab );
    ULONG n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        SvMetaClass * pCl = pEle->GetClass();
        WriteTab( rOutStm, nTab );
        rOutStm << SvHash_import()->GetName().GetBuffer() << ' '
                << pEle->GetPrefix().GetBuffer();
        if( pEle->GetAutomation() )
            rOutStm << " [ " << SvHash_Automation()->GetName().GetBuffer()
                    << " ]";
        if( pEle->GetPrefix().Len() )
            rOutStm << ' ' << pEle->GetPrefix().GetBuffer();
        rOutStm << ';' << endl;
    }
}

/*************************************************************************
|*    SvMetaClass::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaClass::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    ULONG nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm ) && GetType() == TYPE_CLASS )
    {
        BOOL bOk = TRUE;
        if( rInStm.Read( ':' ) )
        {
            aSuperClass = rBase.ReadKnownClass( rInStm );
            bOk = aSuperClass.Is();
            if( !bOk )
            {
                // Fehler setzen
                rBase.SetError( "unknown super class",
                                rInStm.GetToken() );
                rBase.WriteError( rInStm );
            }
        }
        if( bOk )
        {
            rBase.Write( '.' );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        if( bOk )
            return bOk;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*    SvMetaClass::TestAttribute()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaClass::TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                 SvMetaAttribute & rAttr ) const
{
    if ( !rAttr.GetRef() && rAttr.IsA( TYPE( SvMetaSlot ) ) )
    {
        DBG_ERROR( "Neuer Slot : " );
        DBG_ERROR( rAttr.GetSlotId().GetBuffer() );
    }

    for( ULONG n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pS = aAttrList.GetObject( n );
        if( pS->GetName() == rAttr.GetName() )
        {
            // Werte muessen uebereinstimmen
            if( pS->GetSlotId().GetValue() != rAttr.GetSlotId().GetValue() )
            {
                DBG_ERROR( "Gleicher Name in MetaClass : " );
                DBG_ERROR( pS->GetName().GetBuffer() );
                DBG_ERROR( pS->GetSlotId().GetBuffer() );
                DBG_ERROR( rAttr.GetSlotId().GetBuffer() );

                ByteString aStr( "Attribute's " );
                aStr += pS->GetName();
                aStr += " with different id's";
                rBase.SetError( aStr, rInStm.GetToken() );
                rBase.WriteError( rInStm );
                return FALSE;
             }
        }
        else
        {
            UINT32 nId1 = pS->GetSlotId().GetValue();
            UINT32 nId2 = rAttr.GetSlotId().GetValue();
            if( nId1 == nId2 && nId1 != 0 /*&& nId2 != 0 ist ueberfluessig*/ )
            {
                DBG_ERROR( "Gleiche Id in MetaClass : " );
                DBG_ERROR( ByteString::CreateFromInt32( pS->GetSlotId().GetValue() ).GetBuffer() );
                DBG_ERROR( pS->GetSlotId().GetBuffer() );
                DBG_ERROR( rAttr.GetSlotId().GetBuffer() );

                ByteString aStr( "Attribute " );
                aStr += pS->GetName();
                aStr += " and Attribute ";
                aStr += rAttr.GetName();
                aStr += " with equal id's";
                rBase.SetError( aStr, rInStm.GetToken() );
                rBase.WriteError( rInStm );
                return FALSE;
             }
        }
    }
    SvMetaClass * pSC = aSuperClass;
    if( pSC )
        return pSC->TestAttribute( rBase, rInStm, rAttr );
    return TRUE;
}

/*************************************************************************
|*    SvMetaClass::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                              USHORT nTab )
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    if( aSuperClass.Is() )
        rOutStm << " : " << aSuperClass->GetName().GetBuffer();
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
}

/*************************************************************************
|*    SvMetaClass::WriteOdlMember()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaClass::WriteOdlMembers( ByteStringList & rSuperList,
                                    BOOL bVariable, BOOL bWriteTab,
                                       SvIdlDataBase & rBase,
                                       SvStream & rOutStm, USHORT nTab )
{
    // alle Attribute schreiben
    ULONG n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );

        ByteString aMangleName = pAttr->GetMangleName( bVariable );
        ByteString * pS = rBase.FindName( aMangleName, rSuperList );

        if( !pS && pAttr->GetExport() )
        {
            // nicht doppelt
            if( bVariable && pAttr->IsVariable() )
            {
                rSuperList.Insert( new ByteString( aMangleName ), LIST_APPEND );
                pAttr->Write( rBase, rOutStm, nTab +1, WRITE_ODL,
                                WA_VARIABLE );
                rOutStm << ';' << endl;
            }
            else if( !bVariable && pAttr->IsMethod() )
            {
                rSuperList.Insert( new ByteString( aMangleName ), LIST_APPEND );
                pAttr->Write( rBase, rOutStm, nTab +1, WRITE_ODL,
                                WA_METHOD );
                rOutStm << ';' << endl;
            }
        }
        else
            continue;
    }
    // alle Attribute der importierten Klassen schreiben
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        SvMetaClass * pCl = pEle->GetClass();
        pCl->WriteOdlMembers( rSuperList, bVariable, bWriteTab,
                                 rBase, rOutStm, nTab );
    }
    // alle Attribute der Superklassen schreiben
    SvMetaClass * pSC = aSuperClass;
    if( pSC )
        pSC->WriteOdlMembers( rSuperList, bVariable, bWriteTab,
                             rBase, rOutStm, nTab );
}
 */

/*************************************************************************
|*    SvMetaClass::Write()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                        USHORT nTab,
                         WriteType nT, WriteAttribute nA )
{
    rBase.aIFaceName = GetName();
    switch( nT )
    {
        case WRITE_ODL:
        {
            DBG_ERROR( "Not supported anymore!" )
/*
            // Schreibt die Attribute
            SvMetaName::Write( rBase, rOutStm, nTab, nT, nA );

            WriteTab( rOutStm, nTab );
            rOutStm << "dispinterface " << GetName().GetBuffer() << endl;
            WriteTab( rOutStm, nTab );
            rOutStm << '{' << endl;

            WriteTab( rOutStm, nTab );
            rOutStm << "properties:";
            rOutStm << endl;

            StringList aSuperList;
            WriteOdlMembers( aSuperList, TRUE, TRUE, rBase, rOutStm, nTab );

            WriteTab( rOutStm, nTab );
            rOutStm << "methods:";
            rOutStm << endl;

            WriteOdlMembers( aSuperList, FALSE, TRUE, rBase, rOutStm, nTab );

            ByteString * pStr = aSuperList.First();
            while( pStr )
            {
                delete pStr;
                pStr = aSuperList.Next();
            }

            WriteTab( rOutStm, 1 );
            rOutStm << '}' << endl;
 */
            break;
        }
        case WRITE_C_SOURCE:
        case WRITE_C_HEADER:
        {
            DBG_ERROR( "Not supported anymore!" )
/*
            StringList aSuperList;
            if( nT == WRITE_C_SOURCE )
            {
                rOutStm << "#pragma code_seg (\"" << GetName().GetBuffer()
                    << "\",\"CODE\")" << endl;
            }
            WriteCFunctions( aSuperList, rBase, rOutStm, nTab, nT );
 */
            break;
        }
        case WRITE_DOCU:
        {
            rOutStm << "<INTERFACE>" << endl
                    << GetName().GetBuffer();
            if ( GetAutomation() )
                rOutStm << " ( Automation ) ";
            rOutStm << endl;
            WriteDescription( rBase, rOutStm );
            rOutStm << "</INTERFACE>" << endl << endl;

            // alle Attribute schreiben
            ULONG n;
            for( n = 0; n < aAttrList.Count(); n++ )
            {
                SvMetaAttribute * pAttr = aAttrList.GetObject( n );
                if( !pAttr->GetHidden() )
                {
                    if( pAttr->IsMethod() )
                        pAttr->Write( rBase, rOutStm, nTab, nT, WA_METHOD );

                    if( pAttr->IsVariable() )
                        pAttr->Write( rBase, rOutStm, nTab, nT, WA_VARIABLE );
                }
            }

            break;
        }
    }
}

/*************************************************************************
|*    SvMetaClass::WriteSlotParamArray()
|*
|*    Beschreibung
*************************************************************************/
USHORT SvMetaClass::WriteSlotParamArray( SvIdlDataBase & rBase,
                                        SvSlotElementList & rSlotList,
                                        SvStream & rOutStm )
{
    USHORT nCount = 0;
    for( ULONG n = 0; n < rSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = rSlotList.GetObject( n );
        SvMetaSlot *pAttr = pEle->xSlot;
        nCount += pAttr->WriteSlotParamArray( rBase, rOutStm );
    }

    return nCount;
}

/*************************************************************************
|*    SvMetaClass::WriteSlots()
|*
|*    Beschreibung
*************************************************************************/
USHORT SvMetaClass::WriteSlots( const ByteString & rShellName,
                                USHORT nCount, SvSlotElementList & rSlotList,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    USHORT nSCount = 0;
    for( ULONG n = 0; n < rSlotList.Count(); n++ )
    {
        rSlotList.Seek(n);
        SvSlotElement * pEle = rSlotList.GetCurObject();
        SvMetaSlot * pAttr = pEle->xSlot;
        nSCount += pAttr->WriteSlotMap( rShellName, nCount + nSCount,
                                        rSlotList, pEle->aPrefix, rBase,
                                        rOutStm );
    }

    return nSCount;
}

/*************************************************************************
|*    SvMetaClass::InsertSlots()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::InsertSlots( SvSlotElementList& rList, SvULongs& rSuperList,
                            SvMetaClassList &rClassList,
                            const ByteString & rPrefix, SvIdlDataBase& rBase)
{
    // Wurde diese Klasse schon geschrieben ?
    if ( rClassList.GetPos(this) != LIST_ENTRY_NOTFOUND )
        return;

    rClassList.Insert(this, LIST_APPEND);

    // alle direkten Attribute schreiben
    ULONG n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );

        ULONG nId = pAttr->GetSlotId().GetValue();
        USHORT nPos;
        for ( nPos=0; nPos < rSuperList.Count(); nPos++ )
        {
            if ( rSuperList.GetObject(nPos) == nId )
                break;
        }

        if( nPos == rSuperList.Count() )
        {
            // nur schreiben, wenn nicht schon bei SubClass oder
            // importiertem Interface geschrieben
            rSuperList.Insert( nId, nPos );
            pAttr->Insert(rList, rPrefix, rBase);
        }
    }

    // Alle schon von SuperShells importierten Interfaces sollen nicht
    // mehr geschrieben werden
    // Es ist also verboten, da\s Shell und SuperShell die gleiche Klasse
    // direkt importieren !
    if( IsShell() && aSuperClass.Is() )
        aSuperClass->FillClasses( rClassList );

    // alle Attribute der importierten Klassen schreiben, sofern diese nicht
    // schon von der Superklasse importiert wurden
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        SvMetaClass * pCl = pEle->GetClass();
        ByteString rPre = rPrefix;
        if( rPre.Len() && pEle->GetPrefix().Len() )
            rPre += '.';
        rPre += pEle->GetPrefix();

        // Zun"achst die direkt importierten Interfaces schreiben
        pCl->InsertSlots( rList, rSuperList, rClassList, rPre, rBase );
    }

    // Superklassen nur schreiben, wenn keine Shell und nicht in der Liste
    if( !IsShell() && aSuperClass.Is() )
    {
        aSuperClass->InsertSlots( rList, rSuperList, rClassList, rPrefix, rBase );
    }
}

/*************************************************************************
|*    SvMetaClass::FillClasses()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::FillClasses( SvMetaClassList & rList )
{
    // Bin ich noch nicht drin ?
    if ( rList.GetPos(this) == LIST_ENTRY_NOTFOUND )
    {
        rList.Insert(this, LIST_APPEND);

        // Meine Imports
        for( ULONG n = 0; n < aClassList.Count(); n++ )
        {
            SvClassElement * pEle = aClassList.GetObject( n );
            SvMetaClass * pCl = pEle->GetClass();
            pCl->FillClasses( rList );
        }

        // Meine Superklasse
        if( aSuperClass.Is() )
            aSuperClass->FillClasses( rList );
    }
}


/*************************************************************************
|*    SvMetaClass::WriteSlotStubs()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteSlotStubs( const ByteString & rShellName,
                                SvSlotElementList & rSlotList,
                                ByteStringList & rList,
                                SvStream & rOutStm )
{
    // alle Attribute schreiben
    for( ULONG n = 0; n < rSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = rSlotList.GetObject( n );
        SvMetaSlot *pAttr = pEle->xSlot;
        pAttr->WriteSlotStubs( rShellName, rList, rOutStm );
    }
}

/*************************************************************************
|*    SvMetaClass::WriteSfx()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    // Klasse definieren
    rOutStm << "#ifdef " << GetName().GetBuffer() << endl;
    rOutStm << "#undef ShellClass" << endl;
    rOutStm << "#undef " << GetName().GetBuffer() << endl;
    rOutStm << "#define ShellClass " << GetName().GetBuffer() << endl;

//    rOutStm << "SFX_TYPELIB(" << GetName().GetBuffer() << ',' << endl
//        << "\t/* library type */"
//        << '"' << ByteString( GetModule()->GetUUId().GetHexName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << "\"," << endl
//        << "\t\"" << GetModule()->GetTypeLibFileName().GetBuffer() << "\","
//        << ByteString::CreateFromInt32( GetModule()->GetVersion().GetMajorVersion() ).GetBuffer() << ','
//        << ByteString::CreateFromInt32( GetModule()->GetVersion().GetMinorVersion() ).GetBuffer() << ',' << endl
//        << "\t/* shell type   */"
//        << '"';
//    if( xAutomationInterface.Is() )
//        rOutStm << ByteString( xAutomationInterface->GetUUId().GetHexName(), RTL_TEXTENCODING_UTF8 ).GetBuffer();
//    else
//        rOutStm << ByteString( GetUUId().GetHexName(), RTL_TEXTENCODING_UTF8 ).GetBuffer();
//    rOutStm << "\");" << endl << endl;

    // Fuer Interfaces werden kein Slotmaps geschrieben
    if( !IsShell() )
    {
        rOutStm << "#endif" << endl << endl;
        return;
    }
    // Parameter Array schreiben
    //rOutStm << "SfxArgList " << GetName().GetBuffer() << "ArgMap[] = {" << endl;
    rOutStm << "SFX_ARGUMENTMAP(" << GetName().GetBuffer() << ')' << endl
        << '{' << endl;

    SvULongs aSuperList;
    SvMetaClassList aClassList;
    SvSlotElementList aSlotList;
    InsertSlots(aSlotList, aSuperList, aClassList, ByteString(), rBase);
    ULONG n;
    for ( n=0; n<aSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = aSlotList.GetObject( n );
        SvMetaSlot *pSlot = pEle->xSlot;
        pSlot->SetListPos(n);
    }

    ULONG nSlotCount = aSlotList.Count();

    // alle Attribute schreiben
    USHORT nArgCount = WriteSlotParamArray( rBase, aSlotList, rOutStm );
    if( nArgCount )
        Back2Delemitter( rOutStm );
    else
    {
        // mindestens einen dummy
        WriteTab( rOutStm, 1 );
        rOutStm << "SFX_ARGUMENT( 0, 0, SfxVoidItem )" << endl;
    }
    rOutStm << endl << "};" << endl << endl;

    ByteStringList aStringList;
    WriteSlotStubs( GetName(), aSlotList, aStringList, rOutStm );
    ByteString * pStr = aStringList.First();
    while( pStr )
    {
        delete pStr;
        pStr = aStringList.Next();
    }

    rOutStm << endl;

    // Slotmap schreiben
    rOutStm << "SFX_SLOTMAP_ARG(" << GetName().GetBuffer() << ')' << endl
        << '{' << endl;

    // alle Attribute schreiben
    WriteSlots( GetName(), 0, aSlotList, rBase, rOutStm );
    if( nSlotCount )
        Back2Delemitter( rOutStm );
    else
    {
        // mindestens einen dummy
        WriteTab( rOutStm, 1 );
        rOutStm << "SFX_SLOT_ARG(" << GetName().GetBuffer()
                << ", 0, 0, "
                << "SFX_STUB_PTR_EXEC_NONE,"
                << "SFX_STUB_PTR_STATE_NONE,"
                << "0, SfxVoidItem, 0, 0, \"\", 0 )" << endl;
    }
    rOutStm << endl << "};" << endl << "#endif" << endl << endl;

    for( n=0; n<aSlotList.Count(); n++ )
    {
        aSlotList.Seek(n);
        SvSlotElement* pEle = aSlotList.GetCurObject();
        SvMetaSlot* pAttr = pEle->xSlot;
        pAttr->ResetSlotPointer();
    }

    for ( n=0; n<aSlotList.Count(); n++ )
        delete aSlotList.GetObject(n);
}

void SvMetaClass::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                            Table* pTable )
{
    for( ULONG n=0; n<aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );
        pAttr->WriteHelpId( rBase, rOutStm, pTable );
    }
}

/*************************************************************************
|*    SvMetaShell::WriteSrc()
*************************************************************************/
void SvMetaClass::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
    for( ULONG n=0; n<aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );
        pAttr->WriteSrc( rBase, rOutStm, pTable );
    }
}

/*************************************************************************
|*    SvMetaClass::WriteHxx()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab )
{
    ByteString aSuperName( "SvDispatch" );
    if( GetSuperClass() )
        aSuperName = GetSuperClass()->GetName();
    const char * pSup = aSuperName.GetBuffer();

    rOutStm
    << "class " << GetSvName().GetBuffer()
    << ": public " << pSup << endl
    << '{' << endl
    << "protected:" << endl
    << "\tvirtual SvGlobalName  GetTypeName() const;" << endl
    << "\tvirtual BOOL          FillTypeLibInfo( SvGlobalName *, USHORT * pMajor," << endl
    << "\t                                       USHORT * pMinor ) const;" << endl
    << "\tvirtual BOOL          FillTypeLibInfo( ByteString * pName, USHORT * pMajor," << endl;
    rOutStm
    << "\t                                       USHORT * pMinor ) const;" << endl
    << "\tvirtual void          Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) = 0;" << endl
    << "public:" << endl
    << "\t static SvGlobalName  ClassName()" << endl
    << "\t                      { return SvGlobalName( " << ByteString( GetUUId().GetctorName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << " ); }" << endl
    << "};" << endl;
}

/*************************************************************************
|*    SvMetaClass::WriteCxx()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab )
{
    ByteString aSuperName( "SvDispatch" );
    if( GetSuperClass() )
        aSuperName = GetSuperClass()->GetName();
    const char * pSup = aSuperName.GetBuffer();

    ByteString aName = GetSvName();
    // GetTypeName
    rOutStm << "SvGlobalName " << aName.GetBuffer() << "::GetTypeName() const" << endl
    << '{' << endl
    << "\treturn ClassName();" << endl
    << '}' << endl;

    SvMetaModule * pMod = GetModule();
    // FillTypeLibInfo
    rOutStm << "BOOL " << aName.GetBuffer() << "::FillTypeLibInfo( SvGlobalName * pGN," << endl
    << "\t                               USHORT * pMajor," << endl
    << "\t                               USHORT * pMinor ) const" << endl
    << '{' << endl
    << "\tSvGlobalName aN( " << ByteString( pMod->GetUUId().GetctorName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << " );" << endl;
    rOutStm << "\t*pGN = aN;" << endl
    << "\t*pMajor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMajorVersion()).GetBuffer() << ';' << endl
    << "\t*pMinor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMinorVersion()).GetBuffer() << ';' << endl
    << "\treturn TRUE;" << endl
    << '}' << endl;

    // FillTypeLibInfo
    rOutStm << "BOOL " << aName.GetBuffer() << "::FillTypeLibInfo( ByteString * pName,"
    << "\t                               USHORT * pMajor," << endl
    << "\t                               USHORT * pMinor ) const" << endl;
    rOutStm << '{' << endl
    << "\t*pName = \"" << pMod->GetTypeLibFileName().GetBuffer()  << "\";" << endl
    << "\t*pMajor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMajorVersion()).GetBuffer() << ';' << endl
    << "\t*pMinor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMinorVersion()).GetBuffer() << ';' << endl
    << "\treturn TRUE;" << endl
    << '}' << endl;

    rOutStm << "void " << aName.GetBuffer() << "::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )" << endl
    << '{' << endl
    << "\t" << pSup << "::Notify( rBC, rHint );" << endl
    << '}' << endl;
}

#endif // IDL_COMPILER

