/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datefunc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:12:35 $
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

//------------------------------------------------------------------
//
// date functions add in
//
//------------------------------------------------------------------

#ifndef _SCA_DATEFUNC_HXX
#define _SCA_DATEFUNC_HXX

#include <string.h>

#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XADDIN_HPP_
#include <com/sun/star/sheet/XAddIn.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCOMPATIBILITYNAMES_HPP_
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_ADDIN_XDATEFUNCTIONS_HPP_
#include <com/sun/star/sheet/addin/XDateFunctions.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_ADDIN_XMISCFUNCTIONS_HPP_
#include <com/sun/star/sheet/addin/XMiscFunctions.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>             // helper for implementations
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _TOOLS_RESARY_HXX
#include <tools/resary.hxx>
#endif

//------------------------------------------------------------------

class ScaList
{
private:
    static const sal_uInt32     nStartSize;
    static const sal_uInt32     nIncrSize;

    void**                      pData;          // pointer array
    sal_uInt32                  nSize;          // array size
    sal_uInt32                  nCount;         // next index to be inserted at
    sal_uInt32                  nCurr;          // current pos for iterations

    void                        _Grow();
    inline void                 Grow();

public:
                                ScaList();
    virtual                     ~ScaList();

    inline sal_uInt32           Count() const       { return nCount; }

    inline const void*          GetObject( sal_uInt32 nIndex ) const
                                    { return (nIndex < nCount) ? pData[ nIndex ] : NULL; }

    inline void*                First() { return nCount ? pData[ nCurr = 0 ] : NULL; }
    inline void*                Next()  { return (nCurr + 1 < nCount) ? pData[ ++nCurr ] : NULL; }

    inline void                 Append( void* pNew );
    void                        Insert( void* pNew, sal_uInt32 nIndex );
};


inline void ScaList::Grow()
{
    if( nCount >= nSize )
        _Grow();
}

inline void ScaList::Append( void* pNew )
{
    Grow();
    pData[ nCount++ ] = pNew;
}


//------------------------------------------------------------------

class ScaStringList : protected ScaList
{
public:
    inline                      ScaStringList() : ScaList() {};
    virtual                     ~ScaStringList();

                                using ScaList::Count;

    inline const ::rtl::OUString* Get( sal_uInt32 nIndex ) const;

    inline ::rtl::OUString*     First();
    inline ::rtl::OUString*     Next();

    sal_Bool                    Contains( const ::rtl::OUString& rSearch ) const;

    using ScaList::Append;
    inline void                 Append( ::rtl::OUString* pNew );
    inline void                 Append( const ::rtl::OUString& rNew );
};


inline const ::rtl::OUString* ScaStringList::Get( sal_uInt32 nIndex ) const
{
    return static_cast< const ::rtl::OUString* >( ScaList::GetObject( nIndex ) );
}

inline ::rtl::OUString* ScaStringList::First()
{
    return static_cast< ::rtl::OUString* >( ScaList::First() );
}

inline ::rtl::OUString* ScaStringList::Next()
{
    return static_cast< ::rtl::OUString* >( ScaList::Next() );
}

inline void ScaStringList::Append( ::rtl::OUString* pNew )
{
    ScaList::Append( pNew );
}

inline void ScaStringList::Append( const ::rtl::OUString& rNew )
{
    ScaList::Append( new ::rtl::OUString( rNew ) );
}


//------------------------------------------------------------------

class ScaResId : public ResId
{
public:
                                ScaResId( sal_uInt16 nResId, ResMgr& rResMgr );
};


//------------------------------------------------------------------

class ScaResStringLoader : public Resource
{
private:
    String                      aStr;

public:
    inline                      ScaResStringLoader( sal_uInt16 nResId, sal_uInt16 nStrId, ResMgr& rResMgr );

    inline const String&        GetString() const   { return aStr; }

};


inline ScaResStringLoader::ScaResStringLoader( sal_uInt16 nResId, sal_uInt16 nStrId, ResMgr& rResMgr ) :
    Resource( ScaResId( nResId, rResMgr ) ),
    aStr( ScaResId( nStrId, rResMgr ) )
{
    FreeResource();
}


//------------------------------------------------------------------

class ScaResStringArrLoader : public Resource
{
private:
    ResStringArray              aStrArray;

public:
    inline                      ScaResStringArrLoader( sal_uInt16 nResId, sal_uInt16 nArrayId, ResMgr& rResMgr );

    inline const ResStringArray& GetStringArray() const { return aStrArray; }
};



inline ScaResStringArrLoader::ScaResStringArrLoader( sal_uInt16 nResId, sal_uInt16 nArrayId, ResMgr& rResMgr ) :
    Resource( ScaResId( nResId, rResMgr ) ),
    aStrArray( ScaResId( nArrayId, rResMgr ) )
{
    FreeResource();
}


//------------------------------------------------------------------

class ScaResPublisher : public Resource
{
public:
    inline                      ScaResPublisher( const ScaResId& rResId ) : Resource( rResId ) {}

    inline sal_Bool             IsAvailableRes( const ResId& rResId ) const
                                    { return Resource::IsAvailableRes( rResId ); }
    inline void                 FreeResource()
                                    { Resource::FreeResource(); }
};


//------------------------------------------------------------------

class ScaFuncRes : public Resource
{
public:
                                ScaFuncRes( ResId& rResId, ResMgr& rResMgr, sal_uInt16 nIndex, ::rtl::OUString& rRet );
};


//------------------------------------------------------------------

enum ScaCategory
{
    ScaCat_AddIn,
    ScaCat_DateTime,
    ScaCat_Text,
    ScaCat_Finance,
    ScaCat_Inf,
    ScaCat_Math,
    ScaCat_Tech
};

struct ScaFuncDataBase
{
    const sal_Char*             pIntName;           // internal name (get***)
    sal_uInt16                  nUINameID;          // resource ID to UI name
    sal_uInt16                  nDescrID;           // resource ID to description, parameter names and ~ description
    sal_uInt16                  nCompListID;        // resource ID to list of valid names
    sal_uInt16                  nParamCount;        // number of named / described parameters
    ScaCategory                 eCat;               // function category
    sal_Bool                    bDouble;            // name already exist in Calc
    sal_Bool                    bWithOpt;           // first parameter is internal
};

class ScaFuncData
{
private:
    ::rtl::OUString             aIntName;           // internal name (get***)
    sal_uInt16                  nUINameID;          // resource ID to UI name
    sal_uInt16                  nDescrID;           // leads also to parameter descriptions!
    sal_uInt16                  nCompListID;        // resource ID to list of valid names
    sal_uInt16                  nParamCount;        // num of parameters
    ScaStringList               aCompList;          // list of all valid names
    ScaCategory                 eCat;               // function category
    sal_Bool                    bDouble;            // name already exist in Calc
    sal_Bool                    bWithOpt;           // first parameter is internal

public:
                                ScaFuncData( const ScaFuncDataBase& rBaseData, ResMgr& rRscMgr );
    virtual                     ~ScaFuncData();

    inline sal_uInt16           GetUINameID() const     { return nUINameID; }
    inline sal_uInt16           GetDescrID() const      { return nDescrID; }
    inline ScaCategory          GetCategory() const     { return eCat; }
    inline sal_Bool             IsDouble() const        { return bDouble; }
    inline sal_Bool             HasIntParam() const     { return bWithOpt; }

    sal_uInt16                  GetStrIndex( sal_uInt16 nParam ) const;
    inline sal_Bool             Is( const ::rtl::OUString& rCompare ) const
                                                    { return aIntName == rCompare; }

    inline const ScaStringList& GetCompNameList() const { return aCompList; }
};


//------------------------------------------------------------------

class ScaFuncDataList : private ScaList
{
    ::rtl::OUString             aLastName;
    sal_uInt32                  nLast;

public:
                                ScaFuncDataList( ResMgr& rResMgr );
    virtual                     ~ScaFuncDataList();

                                using ScaList::Count;

    inline const ScaFuncData*   Get( sal_uInt32 nIndex ) const;
    const ScaFuncData*          Get( const ::rtl::OUString& rProgrammaticName ) const;
    inline ScaFuncData*         First();
    inline ScaFuncData*         Next();

    using ScaList::Append;
    inline void                 Append( ScaFuncData* pNew ) { ScaList::Append( pNew ); }
};


inline const ScaFuncData* ScaFuncDataList::Get( sal_uInt32 nIndex ) const
{
    return static_cast< const ScaFuncData* >( ScaList::GetObject( nIndex ) );
}

inline ScaFuncData* ScaFuncDataList::First()
{
    return static_cast< ScaFuncData* >( ScaList::First() );
}

inline ScaFuncData* ScaFuncDataList::Next()
{
    return static_cast< ScaFuncData* >( ScaList::Next() );
}


//------------------------------------------------------------------
//------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DateFunctionAddIn_CreateInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );


// THE AddIn class for date functions

class ScaDateAddIn : public ::cppu::WeakImplHelper6<
                                ::com::sun::star::sheet::XAddIn,
                                ::com::sun::star::sheet::XCompatibilityNames,
                                ::com::sun::star::sheet::addin::XDateFunctions,
                                ::com::sun::star::sheet::addin::XMiscFunctions,
                                ::com::sun::star::lang::XServiceName,
                                ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::lang::Locale  aFuncLoc;
    ::com::sun::star::lang::Locale* pDefLocales;
    ResMgr*                     pResMgr;
    ScaFuncDataList*            pFuncDataList;


    void                        InitDefLocales();
    const ::com::sun::star::lang::Locale& GetLocale( sal_uInt32 nIndex );
    ResMgr&                     GetResMgr() throw( ::com::sun::star::uno::RuntimeException );
    void                        InitData();

    ::rtl::OUString             GetDisplFuncStr( sal_uInt16 nResId ) throw( ::com::sun::star::uno::RuntimeException );
    ::rtl::OUString             GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) throw( ::com::sun::star::uno::RuntimeException );

public:
                                ScaDateAddIn();
    virtual                     ~ScaDateAddIn();

    static ::rtl::OUString      getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

                                // XAddIn
    virtual ::rtl::OUString SAL_CALL getProgrammaticFuntionName( const ::rtl::OUString& aDisplayName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDisplayFunctionName( const ::rtl::OUString& aProgrammaticName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getFunctionDescription( const ::rtl::OUString& aProgrammaticName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDisplayArgumentName( const ::rtl::OUString& aProgrammaticName, sal_Int32 nArgument ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getArgumentDescription( const ::rtl::OUString& aProgrammaticName, sal_Int32 nArgument ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getProgrammaticCategoryName( const ::rtl::OUString& aProgrammaticName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDisplayCategoryName( const ::rtl::OUString& aProgrammaticName ) throw( ::com::sun::star::uno::RuntimeException );

                                // XCompatibilityNames
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::LocalizedName > SAL_CALL getCompatibilityNames( const ::rtl::OUString& aProgrammaticName ) throw( ::com::sun::star::uno::RuntimeException );

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const ::com::sun::star::lang::Locale& eLocale ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale() throw( ::com::sun::star::uno::RuntimeException );

                                // XServiceName
    virtual ::rtl::OUString SAL_CALL getServiceName() throw( ::com::sun::star::uno::RuntimeException );

                                // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL   supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    //  methods from own interfaces start here

                                // XDateFunctions
    virtual sal_Int32 SAL_CALL  getDiffWeeks(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nEndDate, sal_Int32 nStartDate,
                                    sal_Int32 nMode )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Int32 SAL_CALL  getDiffMonths(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nEndDate, sal_Int32 nStartDate,
                                    sal_Int32 nMode )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Int32 SAL_CALL  getDiffYears(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nEndDate, sal_Int32 nStartDate,
                                    sal_Int32 nMode )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Int32 SAL_CALL  getIsLeapYear(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Int32 SAL_CALL  getDaysInMonth(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Int32 SAL_CALL  getDaysInYear(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Int32 SAL_CALL  getWeeksInYear(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    sal_Int32 nDate )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                // XMiscFunctions
    virtual ::rtl::OUString SAL_CALL getRot13(
                                    const ::rtl::OUString& aSrcText )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
};

//------------------------------------------------------------------

#endif  // _SCA_DATEFUNC_HXX

