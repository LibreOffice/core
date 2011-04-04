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
#include "precompiled_basic.hxx"
#include <vcl/msgbox.hxx>
#include <tools/fsys.hxx>

#include "errobject.hxx"
#include "runtime.hxx"
#include "sbintern.hxx"
#include "iosys.hxx"
#include <sb.hrc>
#include <basrid.hxx>
#include "sbunoobj.hxx"
#include "image.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/SearchOptions.hdl>
#include <vcl/svapp.hxx>
#include <unotools/textsearch.hxx>

Reference< XInterface > createComListener( const Any& aControlAny, const ::rtl::OUString& aVBAType,
                                           const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj );

#include <algorithm>
#include <hash_map>

SbxVariable* getDefaultProp( SbxVariable* pRef );

void SbiRuntime::StepNOP()
{}

void SbiRuntime::StepArith( SbxOperator eOp )
{
    SbxVariableRef p1 = PopVar();
    TOSMakeTemp();
    SbxVariable* p2 = GetTOS();


    // This could & should be moved to the MakeTempTOS() method in runtime.cxx
    // In the code which this is cut'npaste from there is a check for a ref
    // count != 1 based on which the copy of the SbxVariable is done.
    // see orig code in MakeTempTOS ( and I'm not sure what the significance,
    // of that is )
    // here we alway seem to have a refcount of 1. Also it seems that
    // MakeTempTOS is called for other operation, so I hold off for now
    // until I have a better idea
    if ( bVBAEnabled
        && ( p2->GetType() == SbxOBJECT || p2->GetType() == SbxVARIANT )
    )
    {
        SbxVariable* pDflt = getDefaultProp( p2 );
        if ( pDflt )
        {
            pDflt->Broadcast( SBX_HINT_DATAWANTED );
            // replacing new p2 on stack causes object pointed by
            // pDft->pParent to be deleted, when p2->Compute() is
            // called below pParent is accessed ( but its deleted )
            // so set it to NULL now
            pDflt->SetParent( NULL );
            p2 = new SbxVariable( *pDflt );
            p2->SetFlag( SBX_READWRITE );
            refExprStk->Put( p2, nExprLvl - 1 );
        }
    }

    p2->ResetFlag( SBX_FIXED );
    p2->Compute( eOp, *p1 );

    checkArithmeticOverflow( p2 );
}

void SbiRuntime::StepUnary( SbxOperator eOp )
{
    TOSMakeTemp();
    SbxVariable* p = GetTOS();
    p->Compute( eOp, *p );
}

void SbiRuntime::StepCompare( SbxOperator eOp )
{
    SbxVariableRef p1 = PopVar();
    SbxVariableRef p2 = PopVar();

    // Make sure objects with default params have
    // values ( and type ) set as appropriate
    SbxDataType p1Type = p1->GetType();
    SbxDataType p2Type = p2->GetType();
    if ( p1Type == p2Type )
    {
        if ( p1Type == SbxEMPTY )
        {
            p1->Broadcast( SBX_HINT_DATAWANTED );
            p2->Broadcast( SBX_HINT_DATAWANTED );
        }
        // if both sides are an object and have default props
        // then we need to use the default props
        // we don't need to worry if only one side ( lhs, rhs ) is an
        // object ( object side will get coerced to correct type in
        // Compare )
        else if ( p1Type ==  SbxOBJECT )
        {
            SbxVariable* pDflt = getDefaultProp( p1 );
            if ( pDflt )
            {
                p1 = pDflt;
                p1->Broadcast( SBX_HINT_DATAWANTED );
            }
            pDflt = getDefaultProp( p2 );
            if ( pDflt )
            {
                p2 = pDflt;
                p2->Broadcast( SBX_HINT_DATAWANTED );
            }
        }

    }
    static SbxVariable* pTRUE = NULL;
    static SbxVariable* pFALSE = NULL;

    if( p2->Compare( eOp, *p1 ) )
    {
        if( !pTRUE )
        {
            pTRUE = new SbxVariable;
            pTRUE->PutBool( sal_True );
            pTRUE->AddRef();
        }
        PushVar( pTRUE );
    }
    else
    {
        if( !pFALSE )
        {
            pFALSE = new SbxVariable;
            pFALSE->PutBool( sal_False );
            pFALSE->AddRef();
        }
        PushVar( pFALSE );
    }
}

void SbiRuntime::StepEXP()      { StepArith( SbxEXP );      }
void SbiRuntime::StepMUL()      { StepArith( SbxMUL );      }
void SbiRuntime::StepDIV()      { StepArith( SbxDIV );      }
void SbiRuntime::StepIDIV()     { StepArith( SbxIDIV );     }
void SbiRuntime::StepMOD()      { StepArith( SbxMOD );      }
void SbiRuntime::StepPLUS()     { StepArith( SbxPLUS );     }
void SbiRuntime::StepMINUS()        { StepArith( SbxMINUS );    }
void SbiRuntime::StepCAT()      { StepArith( SbxCAT );      }
void SbiRuntime::StepAND()      { StepArith( SbxAND );      }
void SbiRuntime::StepOR()       { StepArith( SbxOR );       }
void SbiRuntime::StepXOR()      { StepArith( SbxXOR );      }
void SbiRuntime::StepEQV()      { StepArith( SbxEQV );      }
void SbiRuntime::StepIMP()      { StepArith( SbxIMP );      }

void SbiRuntime::StepNEG()      { StepUnary( SbxNEG );      }
void SbiRuntime::StepNOT()      { StepUnary( SbxNOT );      }

void SbiRuntime::StepEQ()       { StepCompare( SbxEQ );     }
void SbiRuntime::StepNE()       { StepCompare( SbxNE );     }
void SbiRuntime::StepLT()       { StepCompare( SbxLT );     }
void SbiRuntime::StepGT()       { StepCompare( SbxGT );     }
void SbiRuntime::StepLE()       { StepCompare( SbxLE );     }
void SbiRuntime::StepGE()       { StepCompare( SbxGE );     }

namespace
{
    bool NeedEsc(sal_Unicode cCode)
    {
        String sEsc(RTL_CONSTASCII_USTRINGPARAM(".^$+\\|{}()"));
        return (STRING_NOTFOUND != sEsc.Search(cCode));
    }

    String VBALikeToRegexp(const String &rIn)
    {
        String sResult;
        const sal_Unicode *start = rIn.GetBuffer();
        const sal_Unicode *end = start + rIn.Len();

        int seenright = 0;

        sResult.Append('^');

        while (start < end)
        {
            switch (*start)
            {
                case '?':
                    sResult.Append('.');
                    start++;
                    break;
                case '*':
                    sResult.Append(String(RTL_CONSTASCII_USTRINGPARAM(".*")));
                    start++;
                    break;
                case '#':
                    sResult.Append(String(RTL_CONSTASCII_USTRINGPARAM("[0-9]")));
                    start++;
                    break;
                case ']':
                    sResult.Append('\\');
                    sResult.Append(*start++);
                    break;
                case '[':
                    sResult.Append(*start++);
                    seenright = 0;
                    while (start < end && !seenright)
                    {
                        switch (*start)
                        {
                            case '[':
                            case '?':
                            case '*':
                            sResult.Append('\\');
                            sResult.Append(*start);
                                break;
                            case ']':
                            sResult.Append(*start);
                                seenright = 1;
                                break;
                            case '!':
                                sResult.Append('^');
                                break;
                            default:
                            if (NeedEsc(*start))
                                    sResult.Append('\\');
                            sResult.Append(*start);
                                break;
                        }
                        start++;
                    }
                    break;
                default:
                    if (NeedEsc(*start))
                        sResult.Append('\\');
                    sResult.Append(*start++);
            }
        }

        sResult.Append('$');

        return sResult;
    }
}

void SbiRuntime::StepLIKE()
{
    SbxVariableRef refVar1 = PopVar();
    SbxVariableRef refVar2 = PopVar();

    String pattern = VBALikeToRegexp(refVar1->GetString());
    String value = refVar2->GetString();

    com::sun::star::util::SearchOptions aSearchOpt;

    aSearchOpt.algorithmType = com::sun::star::util::SearchAlgorithms_REGEXP;

    aSearchOpt.Locale = Application::GetSettings().GetLocale();
    aSearchOpt.searchString = pattern;

    int bTextMode(1);
    bool bCompatibility = ( pINST && pINST->IsCompatibility() );
    if( bCompatibility )
        bTextMode = GetImageFlag( SBIMG_COMPARETEXT );

    if( bTextMode )
        aSearchOpt.transliterateFlags |= com::sun::star::i18n::TransliterationModules_IGNORE_CASE;

    SbxVariable* pRes = new SbxVariable;
    utl::TextSearch aSearch(aSearchOpt);
    xub_StrLen nStart=0, nEnd=value.Len();
    int bRes = aSearch.SearchFrwrd(value, &nStart, &nEnd);
    pRes->PutBool( bRes != 0 );

    PushVar( pRes );
}

// TOS und TOS-1 sind beides Objektvariable und enthalten den selben Pointer

void SbiRuntime::StepIS()
{
    SbxVariableRef refVar1 = PopVar();
    SbxVariableRef refVar2 = PopVar();

    SbxDataType eType1 = refVar1->GetType();
    SbxDataType eType2 = refVar2->GetType();
    if ( eType1 == SbxEMPTY )
    {
        refVar1->Broadcast( SBX_HINT_DATAWANTED );
        eType1 = refVar1->GetType();
    }
    if ( eType2 == SbxEMPTY )
    {
        refVar2->Broadcast( SBX_HINT_DATAWANTED );
        eType2 = refVar2->GetType();
    }

    sal_Bool bRes = sal_Bool( eType1 == SbxOBJECT && eType2 == SbxOBJECT );
    if ( bVBAEnabled  && !bRes )
        Error( SbERR_INVALID_USAGE_OBJECT );
    bRes = ( bRes && refVar1->GetObject() == refVar2->GetObject() );
    SbxVariable* pRes = new SbxVariable;
    pRes->PutBool( bRes );
    PushVar( pRes );
}

// Aktualisieren des Wertes von TOS

void SbiRuntime::StepGET()
{
    SbxVariable* p = GetTOS();
    p->Broadcast( SBX_HINT_DATAWANTED );
}

// #67607 Uno-Structs kopieren
inline void checkUnoStructCopy( SbxVariableRef& refVal, SbxVariableRef& refVar )
{
    SbxDataType eVarType = refVar->GetType();
    if( eVarType != SbxOBJECT )
        return;

    SbxObjectRef xValObj = (SbxObject*)refVal->GetObject();
    if( !xValObj.Is() || xValObj->ISA(SbUnoAnyObject) )
        return;

    // #115826: Exclude ProcedureProperties to avoid call to Property Get procedure
    if( refVar->ISA(SbProcedureProperty) )
        return;

    SbxObjectRef xVarObj = (SbxObject*)refVar->GetObject();
    SbxDataType eValType = refVal->GetType();
    if( eValType == SbxOBJECT && xVarObj == xValObj )
    {
        SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)xVarObj);
        if( pUnoObj )
        {
            Any aAny = pUnoObj->getUnoAny();
            if( aAny.getValueType().getTypeClass() == TypeClass_STRUCT )
            {
                SbUnoObject* pNewUnoObj = new SbUnoObject( pUnoObj->GetName(), aAny );
                // #70324: ClassName uebernehmen
                pNewUnoObj->SetClassName( pUnoObj->GetClassName() );
                refVar->PutObject( pNewUnoObj );
            }
        }
    }
}


// Ablage von TOS in TOS-1

void SbiRuntime::StepPUT()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // Store auf die eigene Methode (innerhalb einer Function)?
    sal_Bool bFlagsChanged = sal_False;
    sal_uInt16 n = 0;
    if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
    {
        bFlagsChanged = sal_True;
        n = refVar->GetFlags();
        refVar->SetFlag( SBX_WRITE );
    }

    // if left side arg is an object or variant and right handside isn't
    // either an object or a variant then try and see if a default
    // property exists.
    // to use e.g. Range{"A1") = 34
    // could equate to Range("A1").Value = 34
    if ( bVBAEnabled )
    {
        if ( refVar->GetType() == SbxOBJECT  )
        {
            SbxVariable* pDflt = getDefaultProp( refVar );
            if ( pDflt )
                refVar = pDflt;
        }
        if (  refVal->GetType() == SbxOBJECT  )
        {
            SbxVariable* pDflt = getDefaultProp( refVal );
            if ( pDflt )
                refVal = pDflt;
        }
    }

    *refVar = *refVal;
    // lhs is a property who's value is currently null
    if ( !bVBAEnabled || ( bVBAEnabled && refVar->GetType() != SbxEMPTY ) )
    // #67607 Uno-Structs kopieren
        checkUnoStructCopy( refVal, refVar );
    if( bFlagsChanged )
        refVar->SetFlags( n );
}


// VBA Dim As New behavior handling, save init object information
struct DimAsNewRecoverItem
{
    String          m_aObjClass;
    String          m_aObjName;
    SbxObject*      m_pObjParent;
    SbModule*       m_pClassModule;

    DimAsNewRecoverItem( void )
        : m_pObjParent( NULL )
        , m_pClassModule( NULL )
    {}

    DimAsNewRecoverItem( const String& rObjClass, const String& rObjName,
        SbxObject* pObjParent, SbModule* pClassModule )
            : m_aObjClass( rObjClass )
            , m_aObjName( rObjName )
            , m_pObjParent( pObjParent )
            , m_pClassModule( pClassModule )
    {}

};


struct SbxVariablePtrHash
{
    size_t operator()( SbxVariable* pVar ) const
        { return (size_t)pVar; }
};

typedef std::hash_map< SbxVariable*, DimAsNewRecoverItem, SbxVariablePtrHash >  DimAsNewRecoverHash;

static DimAsNewRecoverHash      GaDimAsNewRecoverHash;

void removeDimAsNewRecoverItem( SbxVariable* pVar )
{
    DimAsNewRecoverHash::iterator it = GaDimAsNewRecoverHash.find( pVar );
    if( it != GaDimAsNewRecoverHash.end() )
        GaDimAsNewRecoverHash.erase( it );
}


// Speichern Objektvariable
// Nicht-Objekt-Variable fuehren zu Fehlern

static const char pCollectionStr[] = "Collection";

void SbiRuntime::StepSET_Impl( SbxVariableRef& refVal, SbxVariableRef& refVar, bool bHandleDefaultProp )
{
    // #67733 Typen mit Array-Flag sind auch ok

    // Check var, !object is no error for sure if, only if type is fixed
    SbxDataType eVarType = refVar->GetType();
    if( !bHandleDefaultProp && eVarType != SbxOBJECT && !(eVarType & SbxARRAY) && refVar->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    // Check value, !object is no error for sure if, only if type is fixed
    SbxDataType eValType = refVal->GetType();
//  bool bGetValObject = false;
    if( !bHandleDefaultProp && eValType != SbxOBJECT && !(eValType & SbxARRAY) && refVal->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    // Getting in here causes problems with objects with default properties
    // if they are SbxEMPTY I guess
    if ( !bHandleDefaultProp || ( bHandleDefaultProp && eValType == SbxOBJECT ) )
    {
    // Auf refVal GetObject fuer Collections ausloesen
        SbxBase* pObjVarObj = refVal->GetObject();
        if( pObjVarObj )
        {
            SbxVariableRef refObjVal = PTR_CAST(SbxObject,pObjVarObj);

            // #67733 Typen mit Array-Flag sind auch ok
            if( refObjVal )
                refVal = refObjVal;
            else if( !(eValType & SbxARRAY) )
                refVal = NULL;
        }
    }

    // #52896 Wenn Uno-Sequences bzw. allgemein Arrays einer als
    // Object deklarierten Variable zugewiesen werden, kann hier
    // refVal ungueltig sein!
    if( !refVal )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
    {
        // Store auf die eigene Methode (innerhalb einer Function)?
        sal_Bool bFlagsChanged = sal_False;
        sal_uInt16 n = 0;
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
        {
            bFlagsChanged = sal_True;
            n = refVar->GetFlags();
            refVar->SetFlag( SBX_WRITE );
        }
        SbProcedureProperty* pProcProperty = PTR_CAST(SbProcedureProperty,(SbxVariable*)refVar);
        if( pProcProperty )
            pProcProperty->setSet( true );

        if ( bHandleDefaultProp )
        {
            // get default properties for lhs & rhs where necessary
            // SbxVariable* defaultProp = NULL; unused variable
            bool bLHSHasDefaultProp = false;
            // LHS try determine if a default prop exists
            if ( refVar->GetType() == SbxOBJECT )
            {
                SbxVariable* pDflt = getDefaultProp( refVar );
                if ( pDflt )
                {
                    refVar = pDflt;
                    bLHSHasDefaultProp = true;
                }
            }
            // RHS only get a default prop is the rhs has one
            if (  refVal->GetType() == SbxOBJECT )
            {
                // check if lhs is a null object
                // if it is then use the object not the default property
                SbxObject* pObj = NULL;


                pObj = PTR_CAST(SbxObject,(SbxVariable*)refVar);

                // calling GetObject on a SbxEMPTY variable raises
                // object not set errors, make sure its an Object
                if ( !pObj && refVar->GetType() == SbxOBJECT )
                {
                    SbxBase* pObjVarObj = refVar->GetObject();
                    pObj = PTR_CAST(SbxObject,pObjVarObj);
                }
                SbxVariable* pDflt = NULL;
                if ( pObj || bLHSHasDefaultProp )
                    // lhs is either a valid object || or has a defaultProp
                    pDflt = getDefaultProp( refVal );
                if ( pDflt )
                    refVal = pDflt;
            }
        }

        // Handle Dim As New
        sal_Bool bDimAsNew = bVBAEnabled && refVar->IsSet( SBX_DIM_AS_NEW );
        SbxBaseRef xPrevVarObj;
        if( bDimAsNew )
            xPrevVarObj = refVar->GetObject();

        // Handle withevents
        sal_Bool bWithEvents = refVar->IsSet( SBX_WITH_EVENTS );
        if ( bWithEvents )
        {
            Reference< XInterface > xComListener;

            SbxBase* pObj = refVal->GetObject();
            SbUnoObject* pUnoObj = (pObj != NULL) ? PTR_CAST(SbUnoObject,pObj) : NULL;
            if( pUnoObj != NULL )
            {
                Any aControlAny = pUnoObj->getUnoAny();
                String aDeclareClassName = refVar->GetDeclareClassName();
                ::rtl::OUString aVBAType = aDeclareClassName;
                ::rtl::OUString aPrefix = refVar->GetName();
                SbxObjectRef xScopeObj = refVar->GetParent();
                xComListener = createComListener( aControlAny, aVBAType, aPrefix, xScopeObj );

                refVal->SetDeclareClassName( aDeclareClassName );
                refVal->SetComListener( xComListener, &rBasic );        // Hold reference
            }

            *refVar = *refVal;
        }
        else
        {
            *refVar = *refVal;
        }

        if ( bDimAsNew )
        {
            if( !refVar->ISA(SbxObject) )
            {
                SbxBase* pValObjBase = refVal->GetObject();
                if( pValObjBase == NULL )
                {
                    if( xPrevVarObj.Is() )
                    {
                        // Object is overwritten with NULL, instantiate init object
                        DimAsNewRecoverHash::iterator it = GaDimAsNewRecoverHash.find( refVar );
                        if( it != GaDimAsNewRecoverHash.end() )
                        {
                            const DimAsNewRecoverItem& rItem = it->second;
                            if( rItem.m_pClassModule != NULL )
                            {
                                SbClassModuleObject* pNewObj = new SbClassModuleObject( rItem.m_pClassModule );
                                pNewObj->SetName( rItem.m_aObjName );
                                pNewObj->SetParent( rItem.m_pObjParent );
                                refVar->PutObject( pNewObj );
                            }
                            else if( rItem.m_aObjClass.EqualsIgnoreCaseAscii( pCollectionStr ) )
                            {
                                BasicCollection* pNewCollection = new BasicCollection( String( RTL_CONSTASCII_USTRINGPARAM(pCollectionStr) ) );
                                pNewCollection->SetName( rItem.m_aObjName );
                                pNewCollection->SetParent( rItem.m_pObjParent );
                                refVar->PutObject( pNewCollection );
                            }
                        }
                    }
                }
                else
                {
                    // Does old value exist?
                    bool bFirstInit = !xPrevVarObj.Is();
                    if( bFirstInit )
                    {
                        // Store information to instantiate object later
                        SbxObject* pValObj = PTR_CAST(SbxObject,pValObjBase);
                        if( pValObj != NULL )
                        {
                            String aObjClass = pValObj->GetClassName();

                            SbClassModuleObject* pClassModuleObj = PTR_CAST(SbClassModuleObject,pValObjBase);
                            if( pClassModuleObj != NULL )
                            {
                                SbModule* pClassModule = pClassModuleObj->getClassModule();
                                GaDimAsNewRecoverHash[refVar] =
                                    DimAsNewRecoverItem( aObjClass, pValObj->GetName(), pValObj->GetParent(), pClassModule );
                            }
                            else if( aObjClass.EqualsIgnoreCaseAscii( "Collection" ) )
                            {
                                GaDimAsNewRecoverHash[refVar] =
                                    DimAsNewRecoverItem( aObjClass, pValObj->GetName(), pValObj->GetParent(), NULL );
                            }
                        }
                    }
                }
            }
        }


        // lhs is a property who's value is currently (Empty e.g. no broadcast yet)
        // in this case if there is a default prop involved the value of the
        // default property may infact be void so the type will also be SbxEMPTY
        // in this case we do not want to call checkUnoStructCopy 'cause that will
        // cause an error also
        if ( !bHandleDefaultProp || ( bHandleDefaultProp && ( refVar->GetType() != SbxEMPTY ) ) )
        // #67607 Uno-Structs kopieren
            checkUnoStructCopy( refVal, refVar );
        if( bFlagsChanged )
            refVar->SetFlags( n );
    }
}

void SbiRuntime::StepSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    StepSET_Impl( refVal, refVar, bVBAEnabled ); // this is really assigment
}

void SbiRuntime::StepVBASET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // don't handle default property
    StepSET_Impl( refVal, refVar, false ); // set obj = something
}


// JSM 07.10.95
void SbiRuntime::StepLSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING
     || refVal->GetType() != SbxSTRING )
        Error( SbERR_INVALID_USAGE_OBJECT );
    else
    {
        // Store auf die eigene Methode (innerhalb einer Function)?
        sal_uInt16 n = refVar->GetFlags();
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
            refVar->SetFlag( SBX_WRITE );
        String aRefVarString = refVar->GetString();
        String aRefValString = refVal->GetString();

        sal_uInt16 nVarStrLen = aRefVarString.Len();
        sal_uInt16 nValStrLen = aRefValString.Len();
        String aNewStr;
        if( nVarStrLen > nValStrLen )
        {
            aRefVarString.Fill(nVarStrLen,' ');
            aNewStr  = aRefValString.Copy( 0, nValStrLen );
            aNewStr += aRefVarString.Copy( nValStrLen, nVarStrLen - nValStrLen );
        }
        else
        {
            aNewStr = aRefValString.Copy( 0, nVarStrLen );
        }

        refVar->PutString( aNewStr );
        refVar->SetFlags( n );
    }
}

// JSM 07.10.95
void SbiRuntime::StepRSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING
     || refVal->GetType() != SbxSTRING )
        Error( SbERR_INVALID_USAGE_OBJECT );
    else
    {
        // Store auf die eigene Methode (innerhalb einer Function)?
        sal_uInt16 n = refVar->GetFlags();
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
            refVar->SetFlag( SBX_WRITE );
        String aRefVarString = refVar->GetString();
        String aRefValString = refVal->GetString();

        sal_uInt16 nPos = 0;
        sal_uInt16 nVarStrLen = aRefVarString.Len();
        if( nVarStrLen > aRefValString.Len() )
        {
            aRefVarString.Fill(nVarStrLen,' ');
            nPos = nVarStrLen - aRefValString.Len();
        }
        aRefVarString  = aRefVarString.Copy( 0, nPos );
        aRefVarString += aRefValString.Copy( 0, nVarStrLen - nPos );
        refVar->PutString(aRefVarString);

        refVar->SetFlags( n );
    }
}

// Ablage von TOS in TOS-1, dann ReadOnly-Bit setzen

void SbiRuntime::StepPUTC()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    refVar->SetFlag( SBX_WRITE );
    *refVar = *refVal;
    refVar->ResetFlag( SBX_WRITE );
    refVar->SetFlag( SBX_CONST );
}

// DIM
// TOS = Variable fuer das Array mit Dimensionsangaben als Parameter

void SbiRuntime::StepDIM()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );
}

// #56204 DIM-Funktionalitaet in Hilfsmethode auslagern (step0.cxx)
void SbiRuntime::DimImpl( SbxVariableRef refVar )
{
    SbxArray* pDims = refVar->GetParameters();
    // Muss eine gerade Anzahl Argumente haben
    // Man denke daran, dass Arg[0] nicht zaehlt!
    if( pDims && !( pDims->Count() & 1 ) )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxDataType eType = refVar->IsFixed() ? refVar->GetType() : SbxVARIANT;
        SbxDimArray* pArray = new SbxDimArray( eType );
        // AB 2.4.1996, auch Arrays ohne Dimensionsangaben zulassen (VB-komp.)
        if( pDims )
        {
            refVar->ResetFlag( SBX_VAR_TO_DIM );

            for( sal_uInt16 i = 1; i < pDims->Count(); )
            {
                sal_Int32 lb = pDims->Get( i++ )->GetLong();
                sal_Int32 ub = pDims->Get( i++ )->GetLong();
                if( ub < lb )
                    Error( SbERR_OUT_OF_RANGE ), ub = lb;
                pArray->AddDim32( lb, ub );
                if ( lb != ub )
                    pArray->setHasFixedSize( true );
            }
        }
        else
        {
            // #62867 Beim Anlegen eines Arrays der Laenge 0 wie bei
            // Uno-Sequences der Laenge 0 eine Dimension anlegen
            pArray->unoAddDim( 0, -1 );
        }
        sal_uInt16 nSavFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->PutObject( pArray );
        refVar->SetFlags( nSavFlags );
        refVar->SetParameters( NULL );
    }
}

// REDIM
// TOS  = Variable fuer das Array
// argv = Dimensionsangaben

void SbiRuntime::StepREDIM()
{
    // Im Moment ist es nichts anderes als Dim, da doppeltes Dim
    // bereits vom Compiler erkannt wird.
    StepDIM();
}


// Helper function for StepREDIMP
void implCopyDimArray( SbxDimArray* pNewArray, SbxDimArray* pOldArray, short nMaxDimIndex,
    short nActualDim, sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32& ri = pActualIndices[nActualDim];
    for( ri = pLowerBounds[nActualDim] ; ri <= pUpperBounds[nActualDim] ; ri++ )
    {
        if( nActualDim < nMaxDimIndex )
        {
            implCopyDimArray( pNewArray, pOldArray, nMaxDimIndex, nActualDim + 1,
                pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pOldArray->Get32( pActualIndices );
            SbxVariable* pDest   = pNewArray->Get32( pActualIndices );
            if( pSource && pDest )
                *pDest = *pSource;
        }
    }
}

// REDIM PRESERVE
// TOS  = Variable fuer das Array
// argv = Dimensionsangaben

void SbiRuntime::StepREDIMP()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );

    // Now check, if we can copy from the old array
    if( refRedimpArray.Is() )
    {
        SbxBase* pElemObj = refVar->GetObject();
        SbxDimArray* pNewArray = PTR_CAST(SbxDimArray,pElemObj);
        SbxDimArray* pOldArray = (SbxDimArray*)(SbxArray*)refRedimpArray;
        if( pNewArray )
        {
            short nDimsNew = pNewArray->GetDims();
            short nDimsOld = pOldArray->GetDims();
            short nDims = nDimsNew;
            sal_Bool bRangeError = sal_False;

            // Store dims to use them for copying later
            sal_Int32* pLowerBounds = new sal_Int32[nDims];
            sal_Int32* pUpperBounds = new sal_Int32[nDims];
            sal_Int32* pActualIndices = new sal_Int32[nDims];

            if( nDimsOld != nDimsNew )
            {
                bRangeError = sal_True;
            }
            else
            {
                // Compare bounds
                for( short i = 1 ; i <= nDims ; i++ )
                {
                    sal_Int32 lBoundNew, uBoundNew;
                    sal_Int32 lBoundOld, uBoundOld;
                    pNewArray->GetDim32( i, lBoundNew, uBoundNew );
                    pOldArray->GetDim32( i, lBoundOld, uBoundOld );

                    /* #69094 Allow all dimensions to be changed
                       although Visual Basic is not able to do so.
                    // All bounds but the last have to be the same
                    if( i < nDims && ( lBoundNew != lBoundOld || uBoundNew != uBoundOld ) )
                    {
                        bRangeError = sal_True;
                        break;
                    }
                    else
                    */
                    {
                        // #69094: if( i == nDims )
                        {
                            lBoundNew = std::max( lBoundNew, lBoundOld );
                            uBoundNew = std::min( uBoundNew, uBoundOld );
                        }
                        short j = i - 1;
                        pActualIndices[j] = pLowerBounds[j] = lBoundNew;
                        pUpperBounds[j] = uBoundNew;
                    }
                }
            }

            if( bRangeError )
            {
                StarBASIC::Error( SbERR_OUT_OF_RANGE );
            }
            else
            {
                // Copy data from old array by going recursively through all dimensions
                // (It would be faster to work on the flat internal data array of an
                // SbyArray but this solution is clearer and easier)
                implCopyDimArray( pNewArray, pOldArray, nDims - 1,
                    0, pActualIndices, pLowerBounds, pUpperBounds );
            }

            delete[] pUpperBounds;
            delete[] pLowerBounds;
            delete[] pActualIndices;
            refRedimpArray = NULL;
        }
    }

    //StarBASIC::FatalError( SbERR_NOT_IMPLEMENTED );
}

// REDIM_COPY
// TOS  = Array-Variable, Reference to array is copied
//        Variable is cleared as in ERASE

void SbiRuntime::StepREDIMP_ERASE()
{
    SbxVariableRef refVar = PopVar();
    SbxDataType eType = refVar->GetType();
    if( eType & SbxARRAY )
    {
        SbxBase* pElemObj = refVar->GetObject();
        SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
        if( pDimArray )
        {
            refRedimpArray = pDimArray;
        }

        // As in ERASE
        sal_uInt16 nSavFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->SetType( SbxDataType(eType & 0x0FFF) );
        refVar->SetFlags( nSavFlags );
        refVar->Clear();
    }
    else
    if( refVar->IsFixed() )
        refVar->Clear();
    else
        refVar->SetType( SbxEMPTY );
}

void lcl_clearImpl( SbxVariableRef& refVar, SbxDataType& eType )
{
    sal_uInt16 nSavFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->SetType( SbxDataType(eType & 0x0FFF) );
    refVar->SetFlags( nSavFlags );
    refVar->Clear();
}

void lcl_eraseImpl( SbxVariableRef& refVar, bool bVBAEnabled )
{
    SbxDataType eType = refVar->GetType();
    if( eType & SbxARRAY )
    {
        if ( bVBAEnabled )
        {
            SbxBase* pElemObj = refVar->GetObject();
            SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
            bool bClearValues = true;
            if( pDimArray )
            {
                if ( pDimArray->hasFixedSize() )
                {
                    // Clear all Value(s)
                    pDimArray->SbxArray::Clear();
                    bClearValues = false;
                }
                else
                    pDimArray->Clear(); // clear Dims
            }
            if ( bClearValues )
            {
                SbxArray* pArray = PTR_CAST(SbxArray,pElemObj);
                if ( pArray )
                    pArray->Clear();
            }
        }
        else
        // AB 2.4.1996
        // Arrays haben bei Erase nach VB ein recht komplexes Verhalten. Hier
        // werden zunaechst nur die Typ-Probleme bei REDIM (#26295) beseitigt:
        // Typ hart auf den Array-Typ setzen, da eine Variable mit Array
        // SbxOBJECT ist. Bei REDIM entsteht dann ein SbxOBJECT-Array und
        // der ursruengliche Typ geht verloren -> Laufzeitfehler
            lcl_clearImpl( refVar, eType );
    }
    else
    if( refVar->IsFixed() )
        refVar->Clear();
    else
        refVar->SetType( SbxEMPTY );
}

// Variable loeschen
// TOS = Variable

void SbiRuntime::StepERASE()
{
    SbxVariableRef refVar = PopVar();
    lcl_eraseImpl( refVar, bVBAEnabled );
}

void SbiRuntime::StepERASE_CLEAR()
{
    SbxVariableRef refVar = PopVar();
    lcl_eraseImpl( refVar, bVBAEnabled );
    SbxDataType eType = refVar->GetType();
    lcl_clearImpl( refVar, eType );
}

void SbiRuntime::StepARRAYACCESS()
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    SbxVariableRef refVar = PopVar();
    refVar->SetParameters( refArgv );
    PopArgv();
    PushVar( CheckArray( refVar ) );
}

void SbiRuntime::StepBYVAL()
{
    // Copy variable on stack to break call by reference
    SbxVariableRef pVar = PopVar();
    SbxDataType t = pVar->GetType();

    SbxVariable* pCopyVar = new SbxVariable( t );
    pCopyVar->SetFlag( SBX_READWRITE );
    *pCopyVar = *pVar;

    PushVar( pCopyVar );
}

// Einrichten eines Argvs
// nOp1 bleibt so -> 1. Element ist Returnwert

void SbiRuntime::StepARGC()
{
    PushArgv();
    refArgv = new SbxArray;
    nArgc = 1;
}

// Speichern eines Arguments in Argv

void SbiRuntime::StepARGV()
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxVariableRef pVal = PopVar();

        // Before fix of #94916:
        // if( pVal->ISA(SbxMethod) || pVal->ISA(SbxProperty) )
        if( pVal->ISA(SbxMethod) || pVal->ISA(SbUnoProperty) || pVal->ISA(SbProcedureProperty) )
        {
            // Methoden und Properties evaluieren!
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc++ );
    }
}

// Input to Variable. Die Variable ist auf TOS und wird
// anschliessend entfernt.

void SbiRuntime::StepINPUT()
{
    String s;
    char ch = 0;
    SbError err;
    // Skip whitespace
    while( ( err = pIosys->GetError() ) == 0 )
    {
        ch = pIosys->Read();
        if( ch != ' ' && ch != '\t' && ch != '\n' )
            break;
    }
    if( !err )
    {
        // Scan until comma or whitespace
        char sep = ( ch == '"' ) ? ch : 0;
        if( sep ) ch = pIosys->Read();
        while( ( err = pIosys->GetError() ) == 0 )
        {
            if( ch == sep )
            {
                ch = pIosys->Read();
                if( ch != sep )
                    break;
            }
            else if( !sep && (ch == ',' || ch == '\n') )
                break;
            s += ch;
            ch = pIosys->Read();
        }
        // skip whitespace
        if( ch == ' ' || ch == '\t' )
          while( ( err = pIosys->GetError() ) == 0 )
        {
            if( ch != ' ' && ch != '\t' && ch != '\n' )
                break;
            ch = pIosys->Read();
        }
    }
    if( !err )
    {
        SbxVariableRef pVar = GetTOS();
        // Zuerst versuchen, die Variable mit einem numerischen Wert
        // zu fuellen, dann mit einem Stringwert
        if( !pVar->IsFixed() || pVar->IsNumeric() )
        {
            sal_uInt16 nLen = 0;
            if( !pVar->Scan( s, &nLen ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            // Der Wert muss komplett eingescant werden
            else if( nLen != s.Len() && !pVar->PutString( s ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            else if( nLen != s.Len() && pVar->IsNumeric() )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
                if( !err )
                    err = SbERR_CONVERSION;
            }
        }
        else
        {
            pVar->PutString( s );
            err = SbxBase::GetError();
            SbxBase::ResetError();
        }
    }
    if( err == SbERR_USER_ABORT )
        Error( err );
    else if( err )
    {
        if( pRestart && !pIosys->GetChannel() )
        {
            BasResId aId( IDS_SBERR_START + 4 );
            String aMsg( aId );

            //****** DONT CHECK IN, TEST ONLY *******
            //****** DONT CHECK IN, TEST ONLY *******
            // ErrorBox( NULL, WB_OK, aMsg ).Execute();
            //****** DONT CHECK IN, TEST ONLY *******
            //****** DONT CHECK IN, TEST ONLY *******

            pCode = pRestart;
        }
        else
            Error( err );
    }
    else
    {
        // pIosys->ResetChannel();
        PopVar();
    }
}

// Line Input to Variable. Die Variable ist auf TOS und wird
// anschliessend entfernt.

void SbiRuntime::StepLINPUT()
{
    ByteString aInput;
    pIosys->Read( aInput );
    Error( pIosys->GetError() );
    SbxVariableRef p = PopVar();
    p->PutString( String( aInput, gsl_getSystemTextEncoding() ) );
    // pIosys->ResetChannel();
}

// Programmende

void SbiRuntime::StepSTOP()
{
    pInst->Stop();
}

// FOR-Variable initialisieren

void SbiRuntime::StepINITFOR()
{
    PushFor();
}

void SbiRuntime::StepINITFOREACH()
{
    PushForEach();
}

// FOR-Variable inkrementieren

void SbiRuntime::StepNEXT()
{
    if( !pForStk )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return;
    }
    if( pForStk->eForType == FOR_TO )
        pForStk->refVar->Compute( SbxPLUS, *pForStk->refInc );
}

// Anfang CASE: TOS in CASE-Stack

void SbiRuntime::StepCASE()
{
    if( !refCaseStk.Is() )
        refCaseStk = new SbxArray;
    SbxVariableRef xVar = PopVar();
    refCaseStk->Put( xVar, refCaseStk->Count() );
}

// Ende CASE: Variable freigeben

void SbiRuntime::StepENDCASE()
{
    if( !refCaseStk || !refCaseStk->Count() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
        refCaseStk->Remove( refCaseStk->Count() - 1 );
}

// Standard-Fehlerbehandlung

void SbiRuntime::StepSTDERROR()
{
    pError = NULL; bError = sal_True;
    pInst->aErrorMsg = String();
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    SbxErrObject::getUnoErrObject()->Clear();
}

void SbiRuntime::StepNOERROR()
{
    pInst->aErrorMsg = String();
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    SbxErrObject::getUnoErrObject()->Clear();
    bError = sal_False;
}

// UP verlassen

void SbiRuntime::StepLEAVE()
{
    bRun = sal_False;
        // If VBA and we are leaving an ErrorHandler then clear the error ( it's been processed )
    if ( bInError && pError )
        SbxErrObject::getUnoErrObject()->Clear();
}

void SbiRuntime::StepCHANNEL()      // TOS = Kanalnummer
{
    SbxVariableRef pChan = PopVar();
    short nChan = pChan->GetInteger();
    pIosys->SetChannel( nChan );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepCHANNEL0()
{
    pIosys->ResetChannel();
}

void SbiRuntime::StepPRINT()        // print TOS
{
    SbxVariableRef p = PopVar();
    String s1 = p->GetString();
    String s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
        s = ' ';    // ein Blank davor
    s += s1;
    ByteString aByteStr( s, gsl_getSystemTextEncoding() );
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepPRINTF()       // print TOS in field
{
    SbxVariableRef p = PopVar();
    String s1 = p->GetString();
    String s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
        s = ' ';    // ein Blank davor
    s += s1;
    s.Expand( 14, ' ' );
    ByteString aByteStr( s, gsl_getSystemTextEncoding() );
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepWRITE()        // write TOS
{
    SbxVariableRef p = PopVar();
    // Muss der String gekapselt werden?
    char ch = 0;
    switch (p->GetType() )
    {
        case SbxSTRING: ch = '"'; break;
        case SbxCURRENCY:
        case SbxBOOL:
        case SbxDATE: ch = '#'; break;
        default: break;
    }
    String s;
    if( ch )
        s += ch;
    s += p->GetString();
    if( ch )
        s += ch;
    ByteString aByteStr( s, gsl_getSystemTextEncoding() );
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepRENAME()       // Rename Tos+1 to Tos
{
    SbxVariableRef pTos1 = PopVar();
    SbxVariableRef pTos  = PopVar();
    String aDest = pTos1->GetString();
    String aSource = pTos->GetString();

    // <-- UCB
    if( hasUno() )
    {
        implStepRenameUCB( aSource, aDest );
    }
    else
    // --> UCB
    {
#ifdef _OLD_FILE_IMPL
        DirEntry aSourceDirEntry( aSource );
        if( aSourceDirEntry.Exists() )
        {
            if( aSourceDirEntry.MoveTo( DirEntry(aDest) ) != FSYS_ERR_OK )
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
        }
        else
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
#else
        implStepRenameOSL( aSource, aDest );
#endif
    }
}

// TOS = Prompt

void SbiRuntime::StepPROMPT()
{
    SbxVariableRef p = PopVar();
    ByteString aStr( p->GetString(), gsl_getSystemTextEncoding() );
    pIosys->SetPrompt( aStr );
}

// Set Restart point

void SbiRuntime::StepRESTART()
{
    pRestart = pCode;
}

// Leerer Ausdruck auf Stack fuer fehlenden Parameter

void SbiRuntime::StepEMPTY()
{
    // #57915 Die Semantik von StepEMPTY() ist die Repraesentation eines fehlenden
    // Arguments. Dies wird in VB durch ein durch den Wert 448 (SbERR_NAMED_NOT_FOUND)
    // vom Typ Error repraesentiert. StepEmpty jetzt muesste besser StepMISSING()
    // heissen, aber der Name wird der Einfachkeit halber beibehalten.
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    xVar->PutErr( 448 );
    PushVar( xVar );
    // ALT: PushVar( new SbxVariable( SbxEMPTY ) );
}

// TOS = Fehlercode

void SbiRuntime::StepERROR()
{
    SbxVariableRef refCode = PopVar();
    sal_uInt16 n = refCode->GetUShort();
    SbError error = StarBASIC::GetSfxFromVBError( n );
    if ( bVBAEnabled )
        pInst->Error( error );
    else
        Error( error );
}

