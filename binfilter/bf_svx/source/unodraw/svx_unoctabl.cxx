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

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <bf_svtools/pathoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#include <cppuhelper/implbase2.hxx>

#include "xtable.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoColorTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    XColorTable*	pTable;

public:
    SvxUnoColorTable() throw();
    virtual	~SvxUnoColorTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    static OUString getImplementationName_Static() throw()
    {
        return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SvxUnoColorTable"));
    }

    static uno::Sequence< OUString >  getSupportedServiceNames_Static(void) throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

SvxUnoColorTable::SvxUnoColorTable() throw()
{
    pTable = new XColorTable( SvtPathOptions().GetPalettePath() );
}

SvxUnoColorTable::~SvxUnoColorTable() throw()
{
    delete pTable;
}

sal_Bool SAL_CALL SvxUnoColorTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

OUString SAL_CALL SvxUnoColorTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoColorTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SvxUnoColorTable::getSupportedServiceNames_Static(void) throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ColorTable" ));
    return aSNS;
}

// XNameContainer
void SAL_CALL SvxUnoColorTable::insertByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( hasByName( aName ) )
        throw container::ElementExistException();

    INT32 nColor = 0;
    if( aElement >>= nColor )
        throw lang::IllegalArgumentException();

    if( pTable )
    {
        XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName  );
        pTable->Insert( pTable->Count(), pEntry );
    }
}

void SAL_CALL SvxUnoColorTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
}

// XNameReplace
void SAL_CALL SvxUnoColorTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    INT32 nColor = 0;
    if( aElement >>= nColor )
        throw lang::IllegalArgumentException();

    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName );
    delete pTable->Replace( nIndex, pEntry );
}

// XNameAccess
uno::Any SAL_CALL SvxUnoColorTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{DBG_BF_ASSERT(0, "STRIP"); 	uno::Any aAny; return aAny;//STRIP001 
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    const long nCount = pTable ? pTable->Count() : 0;

    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    for( long nIndex = 0; nIndex < nCount; nIndex++ )
    {DBG_BF_ASSERT(0, "STRIP");//STRIP001 
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoColorTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    long nIndex = pTable ? ((XPropertyTable*)pTable)->Get( aName ) : -1;
    return nIndex != -1;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoColorTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const sal_Int32*)0);
}

sal_Bool SAL_CALL SvxUnoColorTable::hasElements(  )
    throw( uno::RuntimeException )
{
    return pTable && pTable->Count() != 0;
}

/**
 * Create a colortable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoColorTable_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw(uno::Exception)
{
    return *new SvxUnoColorTable();
}

//
// export this service
//
}  

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
namespace binfilter {//STRIP009
//STRIP002 added begin 
//------the following is added for sfx2 integrate with svx
extern "C" {
void* SAL_CALL sfx2_component_getFactory(	const	sal_Char*	pImplementationName	,
                                                 void*		pServiceManager		,
                                                 void*		pRegistryKey		);


//----------------------the following part is copied from sfx2 project source\appl\appuno.cxx

sal_Bool SAL_CALL sfx2_component_writeInfo(	void*	pServiceManager	,
                                        void*	pRegistryKey	);
}
//STRIP002 added end 

extern "C"
{

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            writeInfo( pKey, SvxUnoColorTable::getImplementationName_Static(), SvxUnoColorTable::getSupportedServiceNames_Static() );
#ifndef SVX_LIGHT
            //writeInfo( pKey, svx::GraphicExporter_getImplementationName(), svx::GraphicExporter_getSupportedServiceNames() );
#endif
            sfx2_component_writeInfo(pServiceManager,pRegistryKey);//STRIP002
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return sal_True;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    if( pServiceManager  )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        
        if( rtl_str_compare( pImplName, "com.sun.star.drawing.SvxUnoColorTable" ) == 0 )
        {
            xFactory = createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                SvxUnoColorTable::getImplementationName_Static(),
                SvxUnoColorTable_createInstance,
                SvxUnoColorTable::getSupportedServiceNames_Static() );
        }
#ifndef SVX_LIGHT
        /*else if( svx::GraphicExporter_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
                svx::GraphicExporter_getImplementationName(),
                svx::GraphicExporter_createInstance,
                svx::GraphicExporter_getSupportedServiceNames() );
        }*/
#endif
        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    if (0==pRet)// added for sfx2 moudle integration with svx	//STRIP002 begin 
    {
        pRet=sfx2_component_getFactory(pImplName,pServiceManager,pRegistryKey);
    }//STRIP002 end 

    return pRet;
}

}


}
