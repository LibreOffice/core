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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idl.hxx"

#include <ctype.h>
#include <stdio.h>

#include <tools/debug.hxx>

#include <attrib.hxx>
#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>

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
    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x08 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "wrong format" );
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
    sal_uInt8 nMask = 0;
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
/*************************************************************************
|*    SvMetaClass::SvMetaClass()
|*
|*    Beschreibung
*************************************************************************/
SvMetaClass::SvMetaClass()
    : aAutomation( sal_True, sal_False )
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

    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "wrong format" );
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
    sal_uInt8 nMask = 0;
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
                                        sal_Bool bVariable )
{
    // alle Attribute der Klasse schreiben
    sal_uLong n ;
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
    FillSbxMemberObject( rBase, pObj, aSuperList, sal_True );
    FillSbxMemberObject( rBase, pObj, aSuperList, sal_False );

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
                                 SvStream & rOutStm, sal_uInt16 nTab )
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
    sal_uInt32  nTokPos = rInStm.Tell();
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
                        xEle->SetAutomation( sal_True );
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

        sal_Bool bOk = sal_False;
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
    sal_uInt16 nTab
)
{
    //SvMetaType::WriteContextSvIdl( rBase, rOutStm, nTab );
    sal_uLong n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
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
sal_Bool SvMetaClass::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uLong nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm ) && GetType() == TYPE_CLASS )
    {
        sal_Bool bOk = sal_True;
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
    return sal_False;
}

/*************************************************************************
|*    SvMetaClass::TestAttribute()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvMetaClass::TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                 SvMetaAttribute & rAttr ) const
{
    if ( !rAttr.GetRef() && rAttr.IsA( TYPE( SvMetaSlot ) ) )
    {
        DBG_ERROR( "Neuer Slot : " );
        DBG_ERROR( rAttr.GetSlotId().GetBuffer() );
    }

    for( sal_uLong n = 0; n < aAttrList.Count(); n++ )
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
                return sal_False;
             }
        }
        else
        {
            sal_uInt32 nId1 = pS->GetSlotId().GetValue();
            sal_uInt32 nId2 = rAttr.GetSlotId().GetValue();
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
                return sal_False;
             }
        }
    }
    SvMetaClass * pSC = aSuperClass;
    if( pSC )
        return pSC->TestAttribute( rBase, rInStm, rAttr );
    return sal_True;
}

/*************************************************************************
|*    SvMetaClass::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaClass::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                              sal_uInt16 nTab )
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
                                    sal_Bool bVariable, sal_Bool bWriteTab,
                                       SvIdlDataBase & rBase,
                                       SvStream & rOutStm, sal_uInt16 nTab )
{
    // alle Attribute schreiben
    sal_uLong n;
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
                        sal_uInt16 nTab,
                         WriteType nT, WriteAttribute )
{
    rBase.aIFaceName = GetName();
    switch( nT )
    {
        case WRITE_ODL:
        {
            DBG_ERROR( "Not supported anymore!" );
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
            WriteOdlMembers( aSuperList, sal_True, sal_True, rBase, rOutStm, nTab );

            WriteTab( rOutStm, nTab );
            rOutStm << "methods:";
            rOutStm << endl;

            WriteOdlMembers( aSuperList, sal_False, sal_True, rBase, rOutStm, nTab );

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
            DBG_ERROR( "Not supported anymore!" );
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
            WriteDescription( rOutStm );
            rOutStm << "</INTERFACE>" << endl << endl;

            // alle Attribute schreiben
            sal_uLong n;
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
        default:
            break;
    }
}

/*************************************************************************
|*    SvMetaClass::WriteSlotParamArray()
|*
|*    Beschreibung
*************************************************************************/
sal_uInt16 SvMetaClass::WriteSlotParamArray( SvIdlDataBase & rBase,
                                        SvSlotElementList & rSlotList,
                                        SvStream & rOutStm )
{
    sal_uInt16 nCount = 0;
    for( sal_uLong n = 0; n < rSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = rSlotList.GetObject( n );
        SvMetaSlot *pAttr = pEle->xSlot;
        nCount = nCount + pAttr->WriteSlotParamArray( rBase, rOutStm );
    }

    return nCount;
}

/*************************************************************************
|*    SvMetaClass::WriteSlots()
|*
|*    Beschreibung
*************************************************************************/
sal_uInt16 SvMetaClass::WriteSlots( const ByteString & rShellName,
                                sal_uInt16 nCount, SvSlotElementList & rSlotList,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    sal_uInt16 nSCount = 0;
    for( sal_uLong n = 0; n < rSlotList.Count(); n++ )
    {
        rSlotList.Seek(n);
        SvSlotElement * pEle = rSlotList.GetCurObject();
        SvMetaSlot * pAttr = pEle->xSlot;
        nSCount = nSCount + pAttr->WriteSlotMap( rShellName, nCount + nSCount,
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
    sal_uLong n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );

        sal_uLong nId = pAttr->GetSlotId().GetValue();
        sal_uInt16 nPos;
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
        for( sal_uLong n = 0; n < aClassList.Count(); n++ )
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
    for( sal_uLong n = 0; n < rSlotList.Count(); n++ )
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
    SvMetaClassList classList;
    SvSlotElementList aSlotList;
    InsertSlots(aSlotList, aSuperList, classList, ByteString(), rBase);
    sal_uLong n;
    for ( n=0; n<aSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = aSlotList.GetObject( n );
        SvMetaSlot *pSlot = pEle->xSlot;
        pSlot->SetListPos(n);
    }

    sal_uLong nSlotCount = aSlotList.Count();

    // alle Attribute schreiben
    sal_uInt16 nArgCount = WriteSlotParamArray( rBase, aSlotList, rOutStm );
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
    for( sal_uLong n=0; n<aAttrList.Count(); n++ )
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
    for( sal_uLong n=0; n<aAttrList.Count(); n++ )
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
void SvMetaClass::WriteHxx( SvIdlDataBase &, SvStream & rOutStm, sal_uInt16 )
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
    << "\tvirtual sal_Bool          FillTypeLibInfo( SvGlobalName *, sal_uInt16 * pMajor," << endl
    << "\t                                       sal_uInt16 * pMinor ) const;" << endl
    << "\tvirtual sal_Bool          FillTypeLibInfo( ByteString * pName, sal_uInt16 * pMajor," << endl;
    rOutStm
    << "\t                                       sal_uInt16 * pMinor ) const;" << endl
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
void SvMetaClass::WriteCxx( SvIdlDataBase &, SvStream & rOutStm, sal_uInt16 )
{
    ByteString aSuperName( "SvDispatch" );
    if( GetSuperClass() )
        aSuperName = GetSuperClass()->GetName();
    const char * pSup = aSuperName.GetBuffer();

    ByteString name = GetSvName();
    // GetTypeName
    rOutStm << "SvGlobalName " << name.GetBuffer() << "::GetTypeName() const" << endl
    << '{' << endl
    << "\treturn ClassName();" << endl
    << '}' << endl;

    SvMetaModule * pMod = GetModule();
    // FillTypeLibInfo
    rOutStm << "sal_Bool " << name.GetBuffer() << "::FillTypeLibInfo( SvGlobalName * pGN," << endl
    << "\t                               sal_uInt16 * pMajor," << endl
    << "\t                               sal_uInt16 * pMinor ) const" << endl
    << '{' << endl
    << "\tSvGlobalName aN( " << ByteString( pMod->GetUUId().GetctorName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << " );" << endl;
    rOutStm << "\t*pGN = aN;" << endl
    << "\t*pMajor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMajorVersion()).GetBuffer() << ';' << endl
    << "\t*pMinor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMinorVersion()).GetBuffer() << ';' << endl
    << "\treturn sal_True;" << endl
    << '}' << endl;

    // FillTypeLibInfo
    rOutStm << "sal_Bool " << name.GetBuffer() << "::FillTypeLibInfo( ByteString * pName,"
    << "\t                               sal_uInt16 * pMajor," << endl
    << "\t                               sal_uInt16 * pMinor ) const" << endl;
    rOutStm << '{' << endl
    << "\t*pName = \"" << pMod->GetTypeLibFileName().GetBuffer()  << "\";" << endl
    << "\t*pMajor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMajorVersion()).GetBuffer() << ';' << endl
    << "\t*pMinor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMinorVersion()).GetBuffer() << ';' << endl
    << "\treturn sal_True;" << endl
    << '}' << endl;

    rOutStm << "void " << name.GetBuffer() << "::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )" << endl
    << '{' << endl
    << "\t" << pSup << "::Notify( rBC, rHint );" << endl
    << '}' << endl;
}

#endif // IDL_COMPILER

