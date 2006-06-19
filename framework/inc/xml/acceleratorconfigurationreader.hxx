/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorconfigurationreader.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:08:26 $
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

#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_
#include <accelerators/acceleratorcache.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
#include <accelerators/keymapping.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef __COM_SUN_STAR_XML_SAX_XLOCATOR_HPP_
#include <com/sun/star/xml/sax/XLocator.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _SALHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

namespace framework{

class AcceleratorConfigurationReader : public  css::xml::sax::XDocumentHandler
                                     , private ThreadHelpBase
                                     , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // const, types

    private:

        //---------------------------------------
        /** @short  classification of XML elements. */
        enum EXMLElement
        {
            E_ELEMENT_ACCELERATORLIST,
            E_ELEMENT_ITEM
        };

        //---------------------------------------
        /** @short  classification of XML attributes. */
        enum EXMLAttribute
        {
            E_ATTRIBUTE_KEYCODE,
            E_ATTRIBUTE_MOD_SHIFT,
            E_ATTRIBUTE_MOD_MOD1,
            E_ATTRIBUTE_MOD_MOD2,
            E_ATTRIBUTE_URL
        };

        //---------------------------------------
        /** @short  some namespace defines */
        enum EAcceleratorXMLNamespace
        {
            E_NAMESPACE_ACCEL,
            E_NAMESPACE_XLINK
        };

    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  needed to read the xml configuration. */
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xReader;

        //---------------------------------------
        /** @short  reference to the outside container, where this
                    reader/writer must work on. */
        AcceleratorCache& m_rContainer;

        //---------------------------------------
        /** @short  used to detect if an accelerator list
                    occures recursive inside xml. */
        sal_Bool m_bInsideAcceleratorList;

        //---------------------------------------
        /** @short  used to detect if an accelerator item
                    occures recursive inside xml. */
        sal_Bool m_bInsideAcceleratorItem;

        //---------------------------------------
        /** @short  is used to map key codes to its
                    string representation.

            @descr  To perform this operatio is
                    created only one times and holded
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

        //---------------------------------------
        /** @short  provide informations abou the parsing state.

            @descr  We use it to find out the line and column, where
                    an error occure.
          */
        css::uno::Reference< css::xml::sax::XLocator > m_xLocator;

/*        SfxAcceleratorItemList& m_aReadAcceleratorList;
*/

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        /** @short  connect this new reader/writer instance
                    to an outside container, which should be used
                    flushed to the underlying XML configuration or
                    filled from there.

            @param  rContainer
                    a reference to the outside container.
          */
        AcceleratorConfigurationReader(AcceleratorCache& rContainer);

        //---------------------------------------
        /** @short  does nothing real ... */
        virtual ~AcceleratorConfigurationReader();

        //---------------------------------------
        // XInterface
        FWK_DECLARE_XINTERFACE

        //---------------------------------------
        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL endDocument()
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL startElement(const ::rtl::OUString&                                      sElement      ,
                                           const css::uno::Reference< css::xml::sax::XAttributeList >& xAttributeList)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL endElement(const ::rtl::OUString& sElement)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL characters(const ::rtl::OUString& sChars)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString& sWhitespaces)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL processingInstruction(const ::rtl::OUString& sTarget,
                                                    const ::rtl::OUString& sData  )
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator >& xLocator)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

    //-------------------------------------------
    // helper

    private:

        //---------------------------------------
        /** TODO document me */
        static EXMLElement implst_classifyElement(const ::rtl::OUString& sElement);

        //---------------------------------------
        /** TODO document me */
        static EXMLAttribute implst_classifyAttribute(const ::rtl::OUString& sAttribute);

        //---------------------------------------
        /** TODO document me */
        ::rtl::OUString implts_getErrorLineString();
};

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
