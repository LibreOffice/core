/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorconfigurationwriter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:51:22 $
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

#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_
#include <accelerators/acceleratorcache.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
#include <accelerators/keymapping.hxx>
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

//_______________________________________________
// other includes

#ifndef _SALHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

namespace framework{

class AcceleratorConfigurationWriter : private ThreadHelpBase
{
    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  needed to write the xml configuration. */
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xConfig;

        //---------------------------------------
        /** @short  reference to the outside container, where this
                    writer must work on. */
        const AcceleratorCache& m_rContainer;

        //---------------------------------------
        /** @short  is used to map key codes to its
                    string representation.

            @descr  To perform this operatio is
                    created only one times and holded
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        /** @short  connect this new writer instance
                    to an outside container, which should be
                    flushed to the underlying XML configuration.

            @param  rContainer
                    a reference to the outside container.

            @param  xConfig
                    used to write the configuration there.
          */
        AcceleratorConfigurationWriter(const AcceleratorCache&                                       rContainer,
                                       const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig   );

        //---------------------------------------
        /** @short  does nothing real ... */
        virtual ~AcceleratorConfigurationWriter();

        //---------------------------------------
        /** @short  TODO */
        virtual void flush();

    //-------------------------------------------
    // helper

    private:

        //---------------------------------------
        /** @short  TODO */
        void impl_ts_writeKeyCommandPair(const css::awt::KeyEvent&                                     aKey    ,
                                         const ::rtl::OUString&                                        sCommand,
                                         const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig );
};

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_
