/*************************************************************************
 *
 *  $RCSfile: mediadescriptor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2004-04-21 11:53:03 $
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

#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#define _COMPHELPER_MEDIADESCRIPTOR_HXX_

//_______________________________________________
// includes

#ifndef __COM_SUN_STAR_IO_XINPUTSTREAM_HPP__
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//_______________________________________________
// namespace

namespace comphelper{

//_______________________________________________
// definitions

/** @short  can be used to work with a <type scope="::com::sun::star::document">MediaDescriptor</type>
            struct.

    @descr  It wraps a ::std::hash_map around the Sequence< css::beans::PropertyValue >, which
            represent the MediaDescriptor item.
            Further this helper defines often used functions (as e.g. open of the required streams,
            consistent checks etcpp.) and it defines all useable property names.

    @attention  This class isnt threadsafe and must be guarded from outside!
 */
class MediaDescriptor : public SequenceAsHashMap
{
    //-------------------------------------------
    // const
    public:

        //---------------------------------------
        /** @short  these methods can be used to get the different property names
                    as static const OUString values.

            @descr  Because definition and declaration of static const class members
                    does not work as expected under windows (under unix it works as well)
                    these way must be used :-(
          */
        static const ::rtl::OUString& PROP_ASTEMPLATE();
        static const ::rtl::OUString& PROP_CHARACTERSET();
        static const ::rtl::OUString& PROP_DEEPDETECTION();
        static const ::rtl::OUString& PROP_DETECTSERVICE();
        static const ::rtl::OUString& PROP_DOCUMENTSERVICE();
        static const ::rtl::OUString& PROP_EXTENSION();
        static const ::rtl::OUString& PROP_FILENAME();
        static const ::rtl::OUString& PROP_FILTERNAME();
        static const ::rtl::OUString& PROP_FILTEROPTIONS();
        static const ::rtl::OUString& PROP_FORMAT();
        static const ::rtl::OUString& PROP_FRAMENAME();
        static const ::rtl::OUString& PROP_HIDDEN();
        static const ::rtl::OUString& PROP_INPUTSTREAM();
        static const ::rtl::OUString& PROP_INTERACTIONHANDLER();
        static const ::rtl::OUString& PROP_JUMPMARK();
        static const ::rtl::OUString& PROP_MACROEXECUTIONMODE();
        static const ::rtl::OUString& PROP_MEDIATYPE();
        static const ::rtl::OUString& PROP_MINIMIZED();
        static const ::rtl::OUString& PROP_OPENNEWVIEW();
        static const ::rtl::OUString& PROP_OUTPUTSTREAM();
        static const ::rtl::OUString& PROP_PATTERN();
        static const ::rtl::OUString& PROP_POSSIZE();
        static const ::rtl::OUString& PROP_POSTDATA();
        static const ::rtl::OUString& PROP_POSTSTRING();
        static const ::rtl::OUString& PROP_PREVIEW();
        static const ::rtl::OUString& PROP_READONLY();
        static const ::rtl::OUString& PROP_REFERRER();
        static const ::rtl::OUString& PROP_SILENT();
        static const ::rtl::OUString& PROP_STATUSINDICATOR();
        static const ::rtl::OUString& PROP_STREAM();
        static const ::rtl::OUString& PROP_TEMPLATENAME();
        static const ::rtl::OUString& PROP_TEMPLATEREGIONNAME();
        static const ::rtl::OUString& PROP_TYPENAME();
        static const ::rtl::OUString& PROP_UCBCONTENT();
        static const ::rtl::OUString& PROP_UPDATEDOCMODE();
        static const ::rtl::OUString& PROP_URL();
        static const ::rtl::OUString& PROP_VERSION();
        static const ::rtl::OUString& PROP_VIEWID();
        static const ::rtl::OUString& PROP_REPAIRPACKAGE();
        static const ::rtl::OUString& PROP_DOCUMENTTITLE();
        static const ::rtl::OUString& PROP_MODEL();

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  these ctors do nothing - excepting that they forward
                    the given parameters to the base class ctors.

            @descr  The ctros must be overwritten to resolve conflicts with
                    the default ctors of the compiler :-(.
         */
        MediaDescriptor();
        MediaDescriptor(const ::com::sun::star::uno::Any& aSource);
        MediaDescriptor(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lSource);
        MediaDescriptor(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& lSource);

        //---------------------------------------
        /** @short  it checks if the descriptor already has a valid
                    InputStream item and creates a new one, if not.

            @descr  This method uses the current items of this MediaDescriptor,
                    to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
                    It creates a seekable stream and put it into the descriptor.

                    A might existing InteractionHandler will be used automaticly,
                    to solve problems!

            @return TRUE, if the stream was already part of the descriptor or could
                    be created as new item. FALSE otherwhise.
         */
        sal_Bool addInputStream();

    //-------------------------------------------
    // helper
    private:

        //---------------------------------------
        /** @short  tries to open a stream by using the given PostData stream.

            @descr  The stream is used directly ...

                    The MediaDescriptor itself is changed inside this method.
                    Means: the stream is added internal and not returned by a value.

            @param  xPostData
                    the PostData stream.

            @return TRUE if the stream could be added successfully.
                    Note: If FALSE is returned, the error was already handled inside!

            @throw  [css::uno::RuntimeException]
                    if the MediaDescriptor seems to be invalid!
         */
        sal_Bool impl_openStreamWithPostData(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xPostData)
            throw(::com::sun::star::uno::RuntimeException);

        //---------------------------------------
        /** @short  tries to open a stream by using the given URL.

            @descr  First it tries to open the content in r/w mode (if its
                    allowed to do so). Only in case its not allowed or it failed
                    the stream will be tried to open in readonly mode.

                    The MediaDescriptor itself is changed inside this method.
                    Means: the stream is added internal and not returned by a value.

            @param  sURL
                    the URL for open.

            @return TRUE if the stream could be added successfully.
                    Note: If FALSE is returned, the error was already handled inside!

            @throw  [css::uno::RuntimeException]
                    if the MediaDescriptor seems to be invalid!
         */
        sal_Bool impl_openStreamWithURL(const ::rtl::OUString& sURL)
            throw(::com::sun::star::uno::RuntimeException);
};

} // namespace comphelper

#endif // _COMPHELPER_MEDIADESCRIPTOR_HXX_
