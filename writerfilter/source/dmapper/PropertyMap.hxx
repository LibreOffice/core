#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#define INCLUDED_DMAPPER_PROPERTYMAP_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYIDS_HXX
#include <PropertyIds.hxx>
#endif
#include <boost/shared_ptr.hpp>
#include <map>
namespace com{namespace sun{namespace star{
    namespace beans{
    struct PropertyValue;
    }
    namespace table{
        struct BorderLine;
    }
    namespace container{
        class XNameAccess;
        class XNameContainer;
    }
}}}

namespace dmapper{
/*-- 15.06.2006 08:22:33---------------------------------------------------

  -----------------------------------------------------------------------*/
typedef std::map < ::rtl::OUString, ::com::sun::star::uno::Any > _PropertyMap;
class PropertyMap : public _PropertyMap
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aValues;

    public:
        PropertyMap();
        virtual ~PropertyMap();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropertyValues();
    void PropertyMap::Insert( PropertyIds eId, const ::com::sun::star::uno::Any& rAny );

    void insert(const boost::shared_ptr<PropertyMap> pMap);

    void Invalidate()
        {
            if(m_aValues.getLength())
                m_aValues.realloc( 0 );
        }
};
typedef boost::shared_ptr<PropertyMap>  PropertyMapPtr;

/*-- 24.07.2006 08:26:33---------------------------------------------------

  -----------------------------------------------------------------------*/
class SectionPropertyMap : public PropertyMap
{
    //'temporarily' the section page settings are imported as page styles
    // empty strings mark page settings as not yet imported
    ::rtl::OUString                                                             sPageStyleNames[3];
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   aPageStyles[3];
//    ::rtl::OUString sFirstPage;
//    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xFirstPage;
//    ::rtl::OUString sRightPage;
//    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xRightPage;
//    ::rtl::OUString sLeftPage;
//    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xLeftPage;

    ::com::sun::star::table::BorderLine*    pBorderLines[4];
    sal_Int32                               nBorderDistances[4];

    public:
        SectionPropertyMap();
        ~SectionPropertyMap();

    enum PageType
    {
        PAGE_FIRST,
        PAGE_LEFT,
        PAGE_RIGHT
    };
    enum BorderPosition
    {
        BORDER_LEFT,
        BORDER_RIGHT,
        BORDER_TOP,
        BORDER_BOTTOM
    };

    const ::rtl::OUString&  GetPageStyleName( PageType eType );
    void                    SetPageStyleName( PageType eType, const ::rtl::OUString& rName);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > GetPageStyle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            PageType eType);

    void SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const ::com::sun::star::table::BorderLine& rBorderLine );
    //determine which style gets the borders
    void ApplyBorderToPageStyles(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xStyles,
            sal_Int32 nValue );
};

} //namespace dmapper
#endif
