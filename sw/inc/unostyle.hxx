/*************************************************************************
 *
 *  $RCSfile: unostyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-06 06:36:02 $
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
#ifndef _UNOSTYLE_HXX
#define _UNOSTYLE_HXX

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLELOADER_HPP_
#include <com/sun/star/style/XStyleLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx> // helper for implementations
#endif

class SwDocShell;
struct SfxItemPropertyMap;
class SwXStyle;
class SwStyleProperties_Impl;
class SwStartNode;
/******************************************************************************
 *
 ******************************************************************************/
/*-----------------12.02.98 08:27-------------------

--------------------------------------------------*/
class SwXStyleFamilies :  public cppu::WeakImplHelper4
<
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::style::XStyleLoader
>,
    public SwUnoCollection
{
    SwDocShell*         pDocShell;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   pxCharStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   pxParaStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   pxFrameStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   pxPageStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   pxNumberingStyles;

    sal_Bool                bLoadStyleText      : 1;
    sal_Bool                bLoadStyleFrame     : 1;
    sal_Bool                bLoadStylePage      : 1;
    sal_Bool                bLoadStyleOverwrite : 1;
    sal_Bool                bLoadStyleNumbering : 1;

public:
    SwXStyleFamilies(SwDocShell& rDocShell);
    virtual ~SwXStyleFamilies();

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XStyleLoader
    virtual void SAL_CALL loadStylesFromURL(const rtl::OUString& rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aOptions) throw( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getStyleLoaderOptions(void) throw( ::com::sun::star::uno::RuntimeException );


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static const String&    GetProgrammaticName(const String& rUIName, SfxStyleFamily eFamily);
    static const String&    GetUIName(const String& rProgrammaticName, SfxStyleFamily eFamily);
};

/* -----------------26.05.98 10:56-------------------
 *
 * --------------------------------------------------*/
class SwXStyleFamily : public cppu::WeakImplHelper3
<
    ::com::sun::star::container::XNameContainer,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XIndexAccess
>,
    public SfxListener
{
    SfxStyleFamily              eFamily;        // Familie
    SfxStyleSheetBasePool*      pBasePool;
//  const SfxItemPropertyMap*   _pPropMap;
    SwDocShell*                 pDocShell;

    SwXStyle*               _FindStyle(const String& rStyleName) const;
public:
    SwXStyleFamily(SwDocShell* pDocShell, sal_uInt16 nFamily);
    ~SwXStyleFamily();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XNameContainer
    virtual void SAL_CALL insertByName(const rtl::OUString& Name, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL replaceByName(const rtl::OUString& Name, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeByName(const rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

   //SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};
/* -----------------20.05.98 09:51-------------------
 *
 * --------------------------------------------------*/
class SwXStyle : public cppu::WeakImplHelper5
<
    ::com::sun::star::style::XStyle,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::beans::XPropertyState
>,
        public SfxListener
{
    friend class SwXStyleFamily;
    SwDoc*                  m_pDoc;
    String                  sStyleName;
    SfxStyleSheetBasePool*  pBasePool;
    SfxStyleFamily          eFamily;    // fuer Notify

    sal_Bool                    bIsDescriptor  : 1;
    sal_Bool                    bIsConditional : 1;
    String                  sParentStyleName;
    SwStyleProperties_Impl* pPropImpl;

    void    ApplyDescriptorProperties();
protected:
    void    Invalidate();

    const SfxStyleSheetBasePool*    GetBasePool() const {return pBasePool;}
    SfxStyleSheetBasePool*  GetBasePool() {return pBasePool;}

    void                SetStyleName(const String& rSet){ sStyleName = rSet;}
public:
    SwXStyle(SfxStyleFamily eFam = SFX_STYLE_FAMILY_PARA, BOOL bConditional = FALSE);
    SwXStyle(SfxStyleSheetBasePool& rPool, SfxStyleFamily eFam,
                                SwDoc*  pDoc,
                                const String& rStyleName);//,
                                    //const SfxItemPropertyMap* _pMap);
    ~SwXStyle();

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XStyle
    virtual sal_Bool SAL_CALL isUserDefined(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isInUse(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL getParentStyle(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setParentStyle(const rtl::OUString& aParentStyle) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    const String&       GetStyleName() const { return sStyleName;}
    SfxStyleFamily      GetFamily() const {return eFamily;}

    sal_Bool                IsDescriptor() const {return bIsDescriptor;}
    sal_Bool                IsConditional() const { return bIsConditional;}
    const String&       GetParentStyleName() const { return sParentStyleName;}
    void                SetDoc(SwDoc* pDc, SfxStyleSheetBasePool*   pPool)
                            {
                                bIsDescriptor = sal_False; m_pDoc = pDc;
                                pBasePool = pPool;
                                StartListening(*pBasePool);
                            }
    const SwDoc*                GetDoc() const { return m_pDoc; }
};

/* -----------------24.08.98 16:04-------------------
 *
 * --------------------------------------------------*/
class SwXPageStyle : public SwXStyle
{
    SwDocShell*     pDocShell;
public:
    SwXPageStyle(SfxStyleSheetBasePool& rPool, SwDocShell* pDocSh, SfxStyleFamily eFam,
                                const String& rStyleName);//,
                                    //const SfxItemPropertyMap* _pMap);
    SwXPageStyle(SwDocShell* pDocSh);
    ~SwXPageStyle();

    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    const SwStartNode* GetStartNode(sal_Bool bHeader, sal_Bool bLeft);
};
#endif


