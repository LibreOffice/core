/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <com/sun/star/util/SearchOptions.hpp>
#include <rtl/instance.hxx>
#include <vcl/svapp.hxx>
#include <unotools/textsearch.hxx>

Reference< XInterface > createComListener( const Any& aControlAny, const ::rtl::OUString& aVBAType,
                                           const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj );

#include <algorithm>
#include <boost/unordered_map.hpp>

// for a patch forward declaring these methods below makes sense
// but, #FIXME lets really just move the methods to the top
void lcl_clearImpl( SbxVariableRef& refVar, SbxDataType& eType );
void lcl_eraseImpl( SbxVariableRef& refVar, bool bVBAEnabled );

SbxVariable* getDefaultProp( SbxVariable* pRef );

void SbiRuntime::StepNOP()
{}

void SbiRuntime::StepArith( SbxOperator eOp )
{
    SbxVariableRef p1 = PopVar();
    TOSMakeTemp();
    SbxVariable* p2 = GetTOS();

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
    if ( p1Type == SbxEMPTY )
    {
        p1->Broadcast( SBX_HINT_DATAWANTED );
        p1Type = p1->GetType();
    }
    if ( p2Type == SbxEMPTY )
    {
        p2->Broadcast( SBX_HINT_DATAWANTED );
        p2Type = p2->GetType();
    }
    if ( p1Type == p2Type )
    {
        // if both sides are an object and have default props
        // then we need to use the default props
        // we don't need to worry if only one side ( lhs, rhs ) is an
        // object ( object side will get coerced to correct type in
        // Compare )
        if ( p1Type ==  SbxOBJECT )
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
    static SbxVariable* pNULL = NULL;
    // why do this on non-windows ?
    // why do this at all ?
    // I dumbly follow the pattern :-/
    if ( bVBAEnabled && ( p1->IsNull() || p2->IsNull() ) )
    {
        if( !pNULL )
        {
            pNULL = new SbxVariable;
            pNULL->PutNull();
            pNULL->AddRef();
        }
        PushVar( pNULL );
    }
    else if( p2->Compare( eOp, *p1 ) )
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
    bool bCompatibility = ( GetSbData()->pInst && GetSbData()->pInst->IsCompatibility() );
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

// TOS and TOS-1 are both object variables and contain the same pointer

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

// update the value of TOS

void SbiRuntime::StepGET()
{
    SbxVariable* p = GetTOS();
    p->Broadcast( SBX_HINT_DATAWANTED );
}

// #67607 copy Uno-Structs
inline bool checkUnoStructCopy( bool bVBA, SbxVariableRef& refVal, SbxVariableRef& refVar )
{
    SbxDataType eVarType = refVar->GetType();
    SbxDataType eValType = refVal->GetType();

    if ( !( !bVBA|| ( bVBA && refVar->GetType() != SbxEMPTY ) ) || !refVar->CanWrite() )
        return false;

    if ( eValType != SbxOBJECT )
        return false;
    // we seem to be duplicating parts of SbxValue=operator, maybe we should just move this to
    // there :-/ not sure if for every '=' we would want struct handling
    if( eVarType != SbxOBJECT )
    {
        if ( refVar->IsFixed() )
            return false;
    }
    // #115826: Exclude ProcedureProperties to avoid call to Property Get procedure
    else if( refVar->ISA(SbProcedureProperty) )
        return false;

    SbxObjectRef xValObj = (SbxObject*)refVal->GetObject();
    if( !xValObj.Is() || xValObj->ISA(SbUnoAnyObject) )
        return false;

    SbUnoObject* pUnoVal =  PTR_CAST(SbUnoObject,(SbxObject*)xValObj);
    SbUnoStructRefObject* pUnoStructVal = PTR_CAST(SbUnoStructRefObject,(SbxObject*)xValObj);
    Any aAny = pUnoVal ? pUnoVal->getUnoAny() : pUnoStructVal->getUnoAny();
    if (  aAny.getValueType().getTypeClass() == TypeClass_STRUCT )
    {
        refVar->SetType( SbxOBJECT );
        SbxObjectRef xVarObj = (SbxObject*)refVar->GetObject();
        SbUnoStructRefObject* pUnoStructObj = PTR_CAST(SbUnoStructRefObject,(SbxObject*)xVarObj);

        if ( ( !pUnoVal && !pUnoStructVal ) )
            return false;

        String sClassName = pUnoVal ? pUnoVal->GetClassName() : pUnoStructVal->GetClassName();
        String sName = pUnoVal ? pUnoVal->GetName() : pUnoStructVal->GetName();

        if ( pUnoStructObj )
        {
            StructRefInfo aInfo = pUnoStructObj->getStructInfo();
            aInfo.setValue( aAny );
        }
        else
        {
            SbUnoObject* pNewUnoObj = new SbUnoObject( sName, aAny );
            // #70324: adopt ClassName
            pNewUnoObj->SetClassName( sClassName );
            refVar->PutObject( pNewUnoObj );
        }
        return true;
    }
    return false;
}


// laying down TOS in TOS-1

void SbiRuntime::StepPUT()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // store on its own method (inside a function)?
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

    if ( !checkUnoStructCopy( bVBAEnabled, refVal, refVar ) )
        *refVar = *refVal;

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

typedef boost::unordered_map< SbxVariable*, DimAsNewRecoverItem,
                              SbxVariablePtrHash >  DimAsNewRecoverHash;

class GaDimAsNewRecoverHash : public rtl::Static<DimAsNewRecoverHash, GaDimAsNewRecoverHash> {};

void removeDimAsNewRecoverItem( SbxVariable* pVar )
{
    DimAsNewRecoverHash &rDimAsNewRecoverHash = GaDimAsNewRecoverHash::get();
    DimAsNewRecoverHash::iterator it = rDimAsNewRecoverHash.find( pVar );
    if( it != rDimAsNewRecoverHash.end() )
        rDimAsNewRecoverHash.erase( it );
}


// saving object variable
// not-object variables will cause errors

static const char pCollectionStr[] = "Collection";

void SbiRuntime::StepSET_Impl( SbxVariableRef& refVal, SbxVariableRef& refVar, bool bHandleDefaultProp )
{
    // #67733 types with array-flag are OK too

    // Check var, !object is no error for sure if, only if type is fixed
    SbxDataType eVarType = refVar->GetType();
    if( !bHandleDefaultProp && eVarType != SbxOBJECT && !(eVarType & SbxARRAY) && refVar->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    // Check value, !object is no error for sure if, only if type is fixed
    SbxDataType eValType = refVal->GetType();
    if( !bHandleDefaultProp && eValType != SbxOBJECT && !(eValType & SbxARRAY) && refVal->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    // Getting in here causes problems with objects with default properties
    // if they are SbxEMPTY I guess
    if ( !bHandleDefaultProp || ( bHandleDefaultProp && eValType == SbxOBJECT ) )
    {
    // activate GetOject for collections on refVal
        SbxBase* pObjVarObj = refVal->GetObject();
        if( pObjVarObj )
        {
            SbxVariableRef refObjVal = PTR_CAST(SbxObject,pObjVarObj);

            if( refObjVal )
                refVal = refObjVal;
            else if( !(eValType & SbxARRAY) )
                refVal = NULL;
        }
    }

    // #52896 refVal can be invalid here, if uno-sequences - or more
    // general arrays - are assigned to variables that are declared
    // as an object!
    if( !refVal )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
    {
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

        }

        // lhs is a property who's value is currently (Empty e.g. no broadcast yet)
        // in this case if there is a default prop involved the value of the
        // default property may infact be void so the type will also be SbxEMPTY
        // in this case we do not want to call checkUnoStructCopy 'cause that will
        // cause an error also
        if ( !checkUnoStructCopy( bHandleDefaultProp, refVal, refVar ) )
            *refVar = *refVal;

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
                        DimAsNewRecoverHash &rDimAsNewRecoverHash = GaDimAsNewRecoverHash::get();
                        DimAsNewRecoverHash::iterator it = rDimAsNewRecoverHash.find( refVar );
                        if( it != rDimAsNewRecoverHash.end() )
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
                            DimAsNewRecoverHash &rDimAsNewRecoverHash = GaDimAsNewRecoverHash::get();
                            if( pClassModuleObj != NULL )
                            {
                                SbModule* pClassModule = pClassModuleObj->getClassModule();
                                rDimAsNewRecoverHash[refVar] =
                                    DimAsNewRecoverItem( aObjClass, pValObj->GetName(), pValObj->GetParent(), pClassModule );
                            }
                            else if( aObjClass.EqualsIgnoreCaseAscii( "Collection" ) )
                            {
                                rDimAsNewRecoverHash[refVar] =
                                    DimAsNewRecoverItem( aObjClass, pValObj->GetName(), pValObj->GetParent(), NULL );
                            }
                        }
                    }
                }
            }
        }


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


void SbiRuntime::StepLSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING
     || refVal->GetType() != SbxSTRING )
        Error( SbERR_INVALID_USAGE_OBJECT );
    else
    {
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

void SbiRuntime::StepRSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING
     || refVal->GetType() != SbxSTRING )
        Error( SbERR_INVALID_USAGE_OBJECT );
    else
    {
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

// laying down TOS in TOS-1, then set ReadOnly-Bit

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
// TOS = variable for the array with dimension information as parameter

void SbiRuntime::StepDIM()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );
}

// #56204 swap out DIM-functionality into a help method (step0.cxx)
void SbiRuntime::DimImpl( SbxVariableRef refVar )
{
    // If refDim then this DIM statement is terminating a ReDIM and
    // previous StepERASE_CLEAR for an array, the following actions have
    // been delayed from ( StepERASE_CLEAR ) 'till here
    if ( refRedim )
    {
        if ( !refRedimpArray ) // only erase the array not ReDim Preserve
            lcl_eraseImpl( refVar, bVBAEnabled );
        SbxDataType eType = refVar->GetType();
        lcl_clearImpl( refVar, eType );
        refRedim = NULL;
    }
    SbxArray* pDims = refVar->GetParameters();
    // must have an even number of arguments
    // have in mind that Arg[0] does not count!
    if( pDims && !( pDims->Count() & 1 ) )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxDataType eType = refVar->IsFixed() ? refVar->GetType() : SbxVARIANT;
        SbxDimArray* pArray = new SbxDimArray( eType );
        // allow arrays without dimension information, too (VB-compatible)
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
            // #62867 On creating an array of the length 0, create
            // a dimension (like for Uno-Sequences of the length 0)
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
// TOS  = variable for the array
// argv = dimension information

void SbiRuntime::StepREDIM()
{
    // Nothing different than dim at the moment because
    // a double dim is already recognized by the compiler.
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
// TOS  = variable for the array
// argv = dimension information

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
                    lBoundNew = std::max( lBoundNew, lBoundOld );
                    uBoundNew = std::min( uBoundNew, uBoundOld );
                    short j = i - 1;
                    pActualIndices[j] = pLowerBounds[j] = lBoundNew;
                    pUpperBounds[j] = uBoundNew;
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

}

// REDIM_COPY
// TOS  = Array-Variable, Reference to array is copied
//        Variable is cleared as in ERASE

void SbiRuntime::StepREDIMP_ERASE()
{
    SbxVariableRef refVar = PopVar();
    refRedim = refVar;
    SbxDataType eType = refVar->GetType();
    if( eType & SbxARRAY )
    {
        SbxBase* pElemObj = refVar->GetObject();
        SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
        if( pDimArray )
        {
            refRedimpArray = pDimArray;
        }

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
        // Arrays have on an erase to VB quite a complex behaviour. Here are
        // only the type problems at REDIM (#26295) removed at first:
        // Set type hard onto the array-type, because a variable with array is
        // SbxOBJECT. At REDIM there's an SbxOBJECT-array generated then and
        // the original type is lost -> runtime error
            lcl_clearImpl( refVar, eType );
    }
    else
    if( refVar->IsFixed() )
        refVar->Clear();
    else
        refVar->SetType( SbxEMPTY );
}

// delete variable
// TOS = variable

void SbiRuntime::StepERASE()
{
    SbxVariableRef refVar = PopVar();
    lcl_eraseImpl( refVar, bVBAEnabled );
}

void SbiRuntime::StepERASE_CLEAR()
{
    refRedim = PopVar();
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

// establishing an argv
// nOp1 stays as it is -> 1st element is the return value

void SbiRuntime::StepARGC()
{
    PushArgv();
    refArgv = new SbxArray;
    nArgc = 1;
}

// storing an argument in Argv

void SbiRuntime::StepARGV()
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxVariableRef pVal = PopVar();

        // Before fix of #94916:
        if( pVal->ISA(SbxMethod) || pVal->ISA(SbUnoProperty) || pVal->ISA(SbProcedureProperty) )
        {
            // evaluate methods and properties!
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc++ );
    }
}

// Input to Variable. The variable is on TOS and is
// is removed afterwards.

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
        // try to fill the variable with a numeric value first,
        // then with a string value
        if( !pVar->IsFixed() || pVar->IsNumeric() )
        {
            sal_uInt16 nLen = 0;
            if( !pVar->Scan( s, &nLen ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            // the value has to be scanned in completely
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
            pCode = pRestart;
        else
            Error( err );
    }
    else
    {
        PopVar();
    }
}

// Line Input to Variable. The variable is on TOS and is
// deleted afterwards.

void SbiRuntime::StepLINPUT()
{
    rtl::OString aInput;
    pIosys->Read( aInput );
    Error( pIosys->GetError() );
    SbxVariableRef p = PopVar();
    p->PutString(rtl::OStringToOUString(aInput, osl_getThreadTextEncoding()));
}

// end of program

void SbiRuntime::StepSTOP()
{
    pInst->Stop();
}


void SbiRuntime::StepINITFOR()
{
    PushFor();
}

void SbiRuntime::StepINITFOREACH()
{
    PushForEach();
}

// increment FOR-variable

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

// beginning CASE: TOS in CASE-stack

void SbiRuntime::StepCASE()
{
    if( !refCaseStk.Is() )
        refCaseStk = new SbxArray;
    SbxVariableRef xVar = PopVar();
    refCaseStk->Put( xVar, refCaseStk->Count() );
}

// end CASE: free variable

void SbiRuntime::StepENDCASE()
{
    if( !refCaseStk || !refCaseStk->Count() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
        refCaseStk->Remove( refCaseStk->Count() - 1 );
}


void SbiRuntime::StepSTDERROR()
{
    pError = NULL; bError = true;
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
    bError = false;
}

// leave UP

void SbiRuntime::StepLEAVE()
{
    bRun = sal_False;
        // If VBA and we are leaving an ErrorHandler then clear the error ( it's been processed )
    if ( bInError && pError )
        SbxErrObject::getUnoErrObject()->Clear();
}

void SbiRuntime::StepCHANNEL()      // TOS = channel number
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
        s = ' ';    // one blank before
    s += s1;
    rtl::OString aByteStr(rtl::OUStringToOString(s, osl_getThreadTextEncoding()));
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepPRINTF()       // print TOS in field
{
    SbxVariableRef p = PopVar();
    String s1 = p->GetString();
    String s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
        s = ' ';
    s += s1;
    s.Expand( 14, ' ' );
    rtl::OString aByteStr(rtl::OUStringToOString(s, osl_getThreadTextEncoding()));
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepWRITE()        // write TOS
{
    SbxVariableRef p = PopVar();
    // Does the string have to be encapsulated?
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
    rtl::OString aByteStr(rtl::OUStringToOString(s, osl_getThreadTextEncoding()));
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepRENAME()       // Rename Tos+1 to Tos
{
    SbxVariableRef pTos1 = PopVar();
    SbxVariableRef pTos  = PopVar();
    String aDest = pTos1->GetString();
    String aSource = pTos->GetString();

    if( hasUno() )
    {
        implStepRenameUCB( aSource, aDest );
    }
    else
    {
        implStepRenameOSL( aSource, aDest );
    }
}

// TOS = Prompt

void SbiRuntime::StepPROMPT()
{
    SbxVariableRef p = PopVar();
    rtl::OString aStr(rtl::OUStringToOString(p->GetString(), osl_getThreadTextEncoding()));
    pIosys->SetPrompt( aStr );
}

// Set Restart point

void SbiRuntime::StepRESTART()
{
    pRestart = pCode;
}

// empty expression on stack for missing parameter

void SbiRuntime::StepEMPTY()
{
    // #57915 The semantics of StepEMPTY() is the representation of a missing argument.
    // This is represented by the value 448 (SbERR_NAMED_NOT_FOUND) of the type error
    // in VB. StepEmpty should now rather be named StepMISSING() but the name is kept
    // to simplify matters.
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    xVar->PutErr( 448 );
    PushVar( xVar );
}

// TOS = error code

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
