#ifndef RPTUI_METADATA_HXX_
#define RPTUI_METADATA_HXX_
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: metadata.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:09:22 $
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

#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYHANDLER_HPP_
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#endif
/** === end UNO includes === **/

//............................................................................
namespace rptui
{
//............................................................................

    struct OPropertyInfoImpl;

    //========================================================================
    //= OPropertyInfoService
    //========================================================================
    class OPropertyInfoService
        :public OModuleClient
    {
        OPropertyInfoService(const OPropertyInfoService&);
        void operator =(const OPropertyInfoService&);
    protected:
        static sal_uInt16               s_nCount;
        static OPropertyInfoImpl*       s_pPropertyInfos;
        // TODO: a real structure which allows quick access by name as well as by id

    public:
        OPropertyInfoService(){}
        virtual ~OPropertyInfoService(){}
        // IPropertyInfoService
        sal_Int32                           getPropertyId(const String& _rName) const;
        String                              getPropertyTranslation(sal_Int32 _nId) const;
        sal_Int32                           getPropertyHelpId(sal_Int32 _nId) const;
        sal_Int16                           getPropertyPos(sal_Int32 _nId) const;
        sal_uInt32                          getPropertyUIFlags(sal_Int32 _nId) const;
        void                                getPropertyEnumRepresentations(sal_Int32 _nId,::std::vector< ::rtl::OUString >& _rOut) const;
        String                              getPropertyName( sal_Int32 _nPropId );
        static void                         getExcludeProperties(::std::vector< com::sun::star::beans::Property >& _rExcludeProperties,const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _xFormComponentHandler);

        bool                                isComposable(
                                                const ::rtl::OUString& _rPropertyName,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >& _xFormComponentHandler
                                            );

    protected:
        static const OPropertyInfoImpl* getPropertyInfo();

        static const OPropertyInfoImpl* getPropertyInfo(const String& _rName);
        static const OPropertyInfoImpl* getPropertyInfo(sal_Int32 _nId);
    };

    //========================================================================
    //= HelpIdUrl
    //========================================================================
    /// small helper to translate help ids into help urls
    class HelpIdUrl
    {
    public:
        static sal_uInt32 getHelpId( const ::rtl::OUString& _rHelpURL );
        static ::rtl::OUString getHelpURL( sal_uInt32 _nHelpId );
    };

    //========================================================================
    //= UI flags (for all browseable properties)
    //========================================================================

#define PROP_FLAG_NONE              0x00000001  // no special flag
#define PROP_FLAG_ENUM              0x00000002  // the property is some kind of enum property, i.e. its
                                                // value is chosen from a fixed list of possible values
#define PROP_FLAG_ENUM_ONE          0x00000004  // the property is an enum property starting with 1
                                                //  (note that this includes PROP_FLAG_ENUM)
#define PROP_FLAG_COMPOSEABLE       0x00000008  // the property is "composeable", i.e. an intersection of property
                                                //  sets should expose it, if all elements do
#define PROP_FLAG_EXPERIMENTAL      0x00000010  // the property is experimental, i.e. should not appear in the
                                                // UI, unless experimental properties are enabled by a configuraiton
                                                // option
#define PROP_FLAG_DATA_PROPERTY     0x00000020  // the property is to appear on the "Data" page

    //========================================================================
    //= property ids (for all browseable properties)
    //========================================================================

    #define PROPERTY_ID_FORCENEWPAGE                    1
    #define PROPERTY_ID_NEWROWORCOL                     2
    #define PROPERTY_ID_KEEPTOGETHER                    3
    #define PROPERTY_ID_CANGROW                         4
    #define PROPERTY_ID_CANSHRINK                       5
    #define PROPERTY_ID_REPEATSECTION                   6
    #define PROPERTY_ID_PRESERVEIRI                     7
    #define PROPERTY_ID_VISIBLE                         8
    #define PROPERTY_ID_GROUPKEEPTOGETHER               9
    #define PROPERTY_ID_PAGEHEADEROPTION                10
    #define PROPERTY_ID_PAGEFOOTEROPTION                11
    #define PROPERTY_ID_POSITIONX                       12
    #define PROPERTY_ID_POSITIONY                       13
    #define PROPERTY_ID_WIDTH                           14
    #define PROPERTY_ID_HEIGHT                          15
    #define PROPERTY_ID_FORMULA                         16
    #define PROPERTY_ID_PRINTREPEATEDVALUES             17
    #define PROPERTY_ID_CONDITIONALPRINTEXPRESSION      18
    #define PROPERTY_ID_INITIALFORMULA                  19
    #define PROPERTY_ID_STARTNEWCOLUMN                  20
    #define PROPERTY_ID_TYPE                            21
    #define PROPERTY_ID_DATAFIELD                       22
    #define PROPERTY_ID_CHARFONTNAME                    23
    #define PROPERTY_ID_PRINTWHENGROUPCHANGE            24
    #define PROPERTY_ID_DEEPTRAVERSING                  25
    #define PROPERTY_ID_PREEVALUATED                    26
    #define PROPERTY_ID_BACKCOLOR                       27
    #define PROPERTY_ID_BACKTRANSPARENT                 28
    #define PROPERTY_ID_CONTROLBACKGROUND               29
    #define PROPERTY_ID_CONTROLBACKGROUNDTRANSPARENT    30
    #define PROPERTY_ID_FORMULALIST                     31
    #define PROPERTY_ID_SCOPE                           32
    #define PROPERTY_ID_RESETPAGENUMBER                 33
    #define PROPERTY_ID_CHARTTYPE                       34
    #define PROPERTY_ID_MASTERFIELDS                    35
    #define PROPERTY_ID_DETAILFIELDS                    36
    #define PROPERTY_ID_PREVIEW_COUNT                   37

//............................................................................
} // namespace rptui
//............................................................................

#endif // RPTUI_METADATA_HXX_

