/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkwrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-04 13:12:15 $
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

#ifndef __ATK_WRAPPER_HXX__
#define __ATK_WRAPPER_HXX__

#include <atk/atk.h>

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

extern "C" {

typedef struct _AtkObjectWrapper      AtkObjectWrapper;
typedef struct _AtkObjectWrapperClass AtkObjectWrapperClass;

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessibleAction;
    class XAccessibleComponent;
    class XAccessibleEditableText;
    class XAccessibleHypertext;
    class XAccessibleImage;
    class XAccessibleSelection;
    class XAccessibleTable;
    class XAccessibleText;
    class XAccessibleTextAttributes;
    class XAccessibleValue;
} } } }


struct _AtkObjectWrapper
{
    AtkObject aParent;

    ::com::sun::star::accessibility::XAccessible               *mpAccessible;
    ::com::sun::star::accessibility::XAccessibleContext        *mpContext;
    ::com::sun::star::accessibility::XAccessibleAction         *mpAction;
    ::com::sun::star::accessibility::XAccessibleComponent      *mpComponent;
    ::com::sun::star::accessibility::XAccessibleEditableText   *mpEditableText;
    ::com::sun::star::accessibility::XAccessibleHypertext      *mpHypertext;
    ::com::sun::star::accessibility::XAccessibleImage          *mpImage;
    ::com::sun::star::accessibility::XAccessibleSelection      *mpSelection;
    ::com::sun::star::accessibility::XAccessibleTable          *mpTable;
    ::com::sun::star::accessibility::XAccessibleText           *mpText;
    ::com::sun::star::accessibility::XAccessibleTextAttributes *mpTextAttributes;
    ::com::sun::star::accessibility::XAccessibleValue          *mpValue;

//    ::rtl::OString * m_pKeyBindings
};

struct _AtkObjectWrapperClass
{
    AtkObjectClass aParentClass;
};

GType                  atk_object_wrapper_get_type (void) G_GNUC_CONST;
AtkObject *            atk_object_wrapper_ref(
    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible,
    bool create = true );

AtkObject *            atk_object_wrapper_new(
    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible,
    AtkObject* parent = NULL );

void                   atk_object_wrapper_dispose(AtkObjectWrapper* wrapper);

AtkStateType mapAtkState( sal_Int16 nState );

void                   actionIfaceInit(AtkActionIface *iface);
void                   componentIfaceInit(AtkComponentIface *iface);
void                   editableTextIfaceInit(AtkEditableTextIface *iface);
void                   hypertextIfaceInit(AtkHypertextIface *iface);
void                   imageIfaceInit(AtkImageIface *iface);
void                   selectionIfaceInit(AtkSelectionIface *iface);
void                   tableIfaceInit(AtkTableIface *iface);
void                   textIfaceInit(AtkTextIface *iface);
void                   valueIfaceInit(AtkValueIface *iface);

} // extern "C"

#define ATK_TYPE_OBJECT_WRAPPER atk_object_wrapper_get_type()
#define ATK_OBJECT_WRAPPER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), ATK_TYPE_OBJECT_WRAPPER, AtkObjectWrapper))

static inline gchar *
OUStringToGChar(const rtl::OUString& rString )
{
    rtl::OString aUtf8 = rtl::OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return g_strdup( aUtf8 );
}

#define OUStringToConstGChar( string ) rtl::OUStringToOString( string, RTL_TEXTENCODING_UTF8 ).getStr()

#endif /* __ATK_WRAPPER_HXX__ */
