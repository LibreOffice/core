/*************************************************************************
 *
 *  $RCSfile: EditBase.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FORMS_EDITBASE_HXX_
#define _FORMS_EDITBASE_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>

// persistence flags for use with the version id
#define PF_HANDLE_COMMON_PROPS  0x8000
    // Derived classes which use their own persistence methods (read/write) and have an own
    // version handling therein may want to clear this flag in getPersistenceFlags.
    // If done so, this class will write an version without a call to writeCommonEditProperties.
#define PF_FAKE_FORMATTED_FIELD 0x4000
    // .... hmmm .... a fake, as the name suggests. see OFormattedFieldWrapper
#define PF_RESERVED_2           0x2000
#define PF_RESERVED_3           0x1000
#define PF_RESERVED_4           0x0800
#define PF_RESERVED_5           0x0400
#define PF_RESERVED_6           0x0200
#define PF_RESERVED_7           0x0100

#define PF_SPECIAL_FLAGS        0xFF00

//.........................................................................
namespace frm
{

//==================================================================
//= OEditBaseModel
//==================================================================
class OEditBaseModel :  public OBoundControlModel
{
    sal_Int16                   m_nLastReadVersion;

protected:
// [properties]         fuer all Editierfelder
    staruno::Any                    m_aDefault;
    ::rtl::OUString                 m_aDefaultText;         // Defaultwert
    sal_Bool                    m_bEmptyIsNull : 1;         // LeerString ist NULL
    sal_Bool                    m_bFilterProposal : 1;      // use a list of possible value in filtermode
// [properties]

    sal_Int16   getLastReadVersion() const { return m_nLastReadVersion; }

public:
    OEditBaseModel(
        const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory,
        const ::rtl::OUString& rUnoControlModelName,
        const ::rtl::OUString& rDefault );

// stario::XPersistObject
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);

// starbeans::XPropertySet
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue,
                                          sal_Int32 nHandle, const staruno::Any& rValue )
                                        throw(starlang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue);

// starbeans::XPropertyState
    virtual starbeans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual staruno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

protected:
    // new properties common to all edit models should be handled with the following two methods
    void SAL_CALL readCommonEditProperties(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);
    void SAL_CALL writeCommonEditProperties(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    void defaultCommonEditProperties();

    virtual sal_Int16 getPersistenceFlags() const;
        // derived classes may use this if they want this base class to write additinal version flags
        // (one of the PF_.... constants). After ::read they may ask for that flags with getLastReadVersion
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_EDITBASE_HXX_

