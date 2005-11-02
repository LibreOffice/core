/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertysethelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:40:50 $
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

#ifndef INCLUDED_CANVAS_PROPERTYSETHELPER_HXX
#define INCLUDED_CANVAS_PROPERTYSETHELPER_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef INCLUDED_CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif

#include <boost/function.hpp>
#include <vector>
#include <memory>


namespace canvas
{
    /** Really simplistic XPropertySet helper for properties.

        This class provides easy access to properties, referenced via
        ASCII strings. The name/property modification callbacks pairs
        are passed into this class via a vector. Each time a property
        is set or queried, the corresponding getter or setter callback
        is called.

        Use this class as a delegate for the corresponding
        XPropertySet methods, and take care of UNO XInterface and lock
        handling by yourself.

        The core responsibility of this this class is the name/value
        mapping for property sets.
     */
    class PropertySetHelper
    {
    public:
        typedef boost::function0< ::com::sun::star::uno::Any >            GetterType;
        typedef boost::function1<void, const ::com::sun::star::uno::Any&> SetterType;
        struct Callbacks
        {
            GetterType getter;
            SetterType setter;
        };
        typedef tools::ValueMap< Callbacks >     MapType;
        typedef std::vector< MapType::MapEntry > InputMap;

        class MakeMap : public InputMap
        {
        public:
            explicit MakeMap(const char*        name,
                             const GetterType&  getter,
                             const SetterType&  setter)
            {
                MapType::MapEntry aEntry={name, {getter, setter}};
                this->push_back(aEntry);
            }
            MakeMap& operator()(const char*        name,
                                const GetterType&  getter,
                                const SetterType&  setter)
            {
                MapType::MapEntry aEntry={name, {getter, setter}};
                this->push_back(aEntry);
                return *this;
            }
        };

        /** Create helper with zero properties
         */
        PropertySetHelper();

        /** Create helper with given name/value map
         */
        explicit PropertySetHelper( const InputMap& rMap );

        /** Init helper with new name/value map

            @param rMap
            Vector of name/function pointers. Each name is offered as
            a property, and reading/writing to this property is passed
            on to the given function pointer.
         */
        void initProperties( const InputMap& rMap );

        /** Add given properties to helper

            @param rMap
            Vector of name/function pointers. Each name is offered as
            a property, and reading/writing to this property is passed
            on to the given function pointer. These name/function
            pairs are added to the already existing ones.
         */
        void addProperties( const InputMap& rMap );

        /** Checks whether the given string corresponds to a valid
            property name.

            @return true, if the given name maps to a known property.
         */
        bool isPropertyName( const ::rtl::OUString& aPropertyName ) const;

        /** Request the currently active map
         */
        const InputMap& getPropertyMap() const { return maMapEntries; }

        // XPropertySet implementation
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > getPropertySetInfo() const;
        void setPropertyValue( const ::rtl::OUString&            aPropertyName,
                               const ::com::sun::star::uno::Any& aValue );
        ::com::sun::star::uno::Any getPropertyValue( const ::rtl::OUString& PropertyName ) const;
        void addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener );
        void removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener );
        void addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener );
        void removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener );

    private:
        std::auto_ptr<MapType>  mpMap;
        InputMap                maMapEntries;
    };
}

#endif /* INCLUDED_CANVAS_PROPERTYSETHELPER_HXX */
