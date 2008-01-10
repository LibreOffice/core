/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokenuno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:10:18 $
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

#ifndef SC_TOKENUNO_HXX
#define SC_TOKENUNO_HXX

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_FORMULATOKEN_HPP_
#include <com/sun/star/sheet/FormulaToken.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XFORMULAPARSER_HPP_
#include <com/sun/star/sheet/XFormulaParser.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XFORMULAOPCODEMAPPER_HPP_
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#endif
#ifndef __com_sun_star_sheet_FormulaOpCodeMapEntry_idl__
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif

class ScTokenArray;
class ScDocShell;


class ScTokenConversion
{
public:
    static bool ConvertToTokenArray(
                        ScTokenArray& rTokenArray,
                        const com::sun::star::uno::Sequence< com::sun::star::sheet::FormulaToken >& rSequence );
    static bool ConvertToTokenSequence(
                        com::sun::star::uno::Sequence< com::sun::star::sheet::FormulaToken >& rSequence,
                        const ScTokenArray& rTokenArray );
};


class ScFormulaParserObj : public ::cppu::WeakImplHelper3<
                            ::com::sun::star::sheet::XFormulaParser,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ::com::sun::star::uno::Sequence< const ::com::sun::star::sheet::FormulaOpCodeMapEntry > maOpCodeMapping;
    ScCompiler::OpCodeMapPtr    mxOpCodeMap;
    ScDocShell*         mpDocShell;
    ScAddress           maRefPos;
    bool                mbEnglish;
    bool                mbR1C1;
    bool                mbComp3D;
    bool                mbIgnoreSpaces;

    void                    SetCompilerFlags( ScCompiler& rCompiler ) const;

public:
                            ScFormulaParserObj(ScDocShell* pDocSh);
    virtual                 ~ScFormulaParserObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XFormulaParser
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL parseFormula(
                                    const ::rtl::OUString& aFormula )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL printFormula( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::FormulaToken >& aTokens )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScFormulaOpCodeMapperObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::sheet::XFormulaOpCodeMapper,
                            ::com::sun::star::lang::XServiceInfo >
{
public:
                            ScFormulaOpCodeMapperObj();
    virtual                 ~ScFormulaOpCodeMapperObj();

                            // XFormulaOpCodeMapper
                            // Attributes
    virtual ::sal_Int32 SAL_CALL getOpCodeExternal() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getOpCodeUnknown() throw (::com::sun::star::uno::RuntimeException);
                            // Methods
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL getMappings(
                                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames,
                                    sal_Int32 nLanguage )
                                throw ( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaOpCodeMapEntry > SAL_CALL getAvailableMappings(
                                    sal_Int32 nLanguage, sal_Int32 nGroups )
                                throw ( ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

};

#endif

