/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nthesimp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:52:29 $
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

#ifndef _LINGU2_THESIMP_HXX_
#define _LINGU2_THESIMP_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase5.hxx> // helper for implementations

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEDISPLAYNAME_HPP_
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>


#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif

#include <unotools/charclass.hxx>

#include <lingutil.hxx>
#include <linguistic/misc.hxx>
#include "ntprophelp.hxx"

#include <osl/file.hxx>
#include "mythes.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

namespace com { namespace sun { namespace star { namespace beans {
        class XPropertySet;
}}}}



///////////////////////////////////////////////////////////////////////////


class Thesaurus :
    public cppu::WeakImplHelper5
    <
        XThesaurus,
        XInitialization,
        XComponent,
        XServiceInfo,
        XServiceDisplayName
    >
{
    Sequence< Locale >                      aSuppLocales;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;
    Reference< XPropertyChangeListener >    xPropHelper;
    PropertyHelper_Thes *                   pPropHelper;
    BOOL                                    bDisposing;
    CharClass **                            aCharSetInfo;
    MyThes **                               aThes;
    rtl_TextEncoding *                      aTEncs;
    Locale *                                aTLocs;
    OUString *                              aTNames;
    sal_Int32                               numthes;




    // disallow copy-constructor and assignment-operator for now
    Thesaurus(const Thesaurus &);
    Thesaurus & operator = (const Thesaurus &);

    PropertyHelper_Thes &   GetPropHelper_Impl();
    PropertyHelper_Thes &   GetPropHelper()
    {
        return pPropHelper ? *pPropHelper : GetPropHelper_Impl();
    }


public:
    Thesaurus();
    virtual ~Thesaurus();

    // XSupportedLocales (for XThesaurus)
    virtual Sequence< Locale > SAL_CALL
        getLocales()
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const Locale& rLocale )
            throw(RuntimeException);

    // XThesaurus
  virtual Sequence< Reference < ::com::sun::star::linguistic2::XMeaning > > SAL_CALL
        queryMeanings( const OUString& rTerm, const Locale& rLocale,
                const PropertyValues& rProperties )
            throw(IllegalArgumentException,
                  RuntimeException);

    // XServiceDisplayName
    virtual OUString SAL_CALL
        getServiceDisplayName( const Locale& rLocale )
            throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL
        initialize( const Sequence< Any >& rArguments )
            throw(Exception, RuntimeException);

    // XComponent
    virtual void SAL_CALL
        dispose()
            throw(RuntimeException);
    virtual void SAL_CALL
        addEventListener( const Reference< XEventListener >& rxListener )
            throw(RuntimeException);
    virtual void SAL_CALL
        removeEventListener( const Reference< XEventListener >& rxListener )
            throw(RuntimeException);

    ////////////////////////////////////////////////////////////
    // Service specific part
    //

    // XServiceInfo
    virtual OUString SAL_CALL
        getImplementationName()
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& rServiceName )
            throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw(RuntimeException);


    static inline OUString
        getImplementationName_Static() throw();
        static Sequence< OUString >
        getSupportedServiceNames_Static() throw();

private:
        sal_uInt16 SAL_CALL capitalType(const OUString&, CharClass *);
        OUString SAL_CALL makeLowerCase(const OUString&, CharClass *);
        OUString SAL_CALL makeUpperCase(const OUString&, CharClass *);
        OUString SAL_CALL makeInitCap(const OUString&, CharClass *);

};

inline OUString Thesaurus::getImplementationName_Static() throw()
{
    return A2OU( "org.openoffice.lingu.new.Thesaurus" );
}


///////////////////////////////////////////////////////////////////////////

#endif
