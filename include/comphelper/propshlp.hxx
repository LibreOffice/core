#pragma once

#include <sal/config.h>

#include <comphelper/comphelperdllapi.h>
#include <cppuhelper/propshlp.hxx>
#include <vector>
#include <unordered_map>

namespace comphelper
{
/**
   You can use this helper class to map a XPropertySet-Interface to a XFast-
   or a XMultiPropertySet interface.
 */
class SAL_WARN_UNUSED COMPHELPER_DLLPUBLIC OPropertyArrayHelper
    : public ::cppu::IPropertyArrayHelper
{
public:
    /**
       Create an object which supports the common property interfaces.
       @param aProps     sequence of properties which are supported by this helper.
                           The sequence aProps should be sorted.
       @param bSorted    indicates that the elements are sorted.
     */
    OPropertyArrayHelper(const std::vector<css::beans::Property>& aProps);

    OPropertyArrayHelper(const css::uno::Sequence<css::beans::Property>& aProps);

    /**
       Return the number of properties.
     */
    sal_Int32 getCount() const { return maInfos.size(); }
    /**
       Return the property members Name and Attribute from the handle nHandle.
       @param nHandle   the handle of a property. If the values of the handles
                          are sorted in the same way as the names and the highest handle value
                          is getCount() -1, then it is only an indexed access to the property array.
                          Otherwise it is a linear search through the array.
       @param pPropName is an out parameter filled with property name of the property with the
                              handle nHandle. May be NULL.
       @param pAttributes is an out parameter filled with attributes of the property with the
                              handle nHandle. May be NULL.
       @return True, if the handle exist, otherwise false.
     */
    virtual sal_Bool fillPropertyMembersByHandle(::rtl::OUString* pPropName, sal_Int16* pAttributes,
                                                 sal_Int32 nHandle) override;
    /**
       Return the sequence of properties. The sequence is sorted by name.
     */
    virtual css::uno::Sequence<css::beans::Property> getProperties() override;
    /**
       Return the property with the name rPropertyName.
       @param rPropertyName the name of the property.
       @exception UnknownPropertyException  thrown if the property name is unknown.
     */
    virtual css::beans::Property getPropertyByName(const ::rtl::OUString& rPropertyName) override;
    /**
       Return true if the property with the name rPropertyName exist, otherwise false.
       @param rPropertyName the name of the property.
     */
    virtual sal_Bool hasPropertyByName(const ::rtl::OUString& rPropertyName) override;
    /**
       Return the handle of the property with the name rPropertyName.
       If the property does not exist -1 is returned.
       @param rPropertyName the name of the property.
     */
    virtual sal_Int32 getHandleByName(const ::rtl::OUString& rPropertyName) override;
    /**
       Fill the array with the handles of the properties.
       @return the handles of the names from the pHandles array. -1
       indicates an unknown property name.
     */
    virtual sal_Int32 fillHandles(
        /*out*/ sal_Int32* pHandles,
        const css::uno::Sequence<::rtl::OUString>& rPropNames) override;

private:
    /** The sequence generated from the pProperties array. */
    css::uno::Sequence<css::beans::Property> maInfos;
    std::unordered_map<OUString, css::beans::Property> maInfosMap;

    /**
       True, If the values of the handles are sorted in the same way as the names
       and the highest handle value is getCount() -1, otherwise false.
     */
    bool mbRightOrdered;
};

} // namespace comphelper
