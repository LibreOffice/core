/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FORMS_EDITBASE_HXX_
#define _FORMS_EDITBASE_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

namespace binfilter {

// persistence flags for use with the version id
#define PF_HANDLE_COMMON_PROPS	0x8000
    // Derived classes which use their own persistence methods (read/write) and have an own
    // version handling therein may want to clear this flag in getPersistenceFlags.
    // If done so, this class will write an version without a call to writeCommonEditProperties.
#define PF_FAKE_FORMATTED_FIELD	0x4000
    // .... hmmm .... a fake, as the name suggests. see OFormattedFieldWrapper
#define PF_RESERVED_2			0x2000
#define PF_RESERVED_3			0x1000
#define PF_RESERVED_4			0x0800
#define PF_RESERVED_5			0x0400
#define PF_RESERVED_6			0x0200
#define PF_RESERVED_7			0x0100

#define PF_SPECIAL_FLAGS		0xFF00

//.........................................................................
namespace frm
{

//==================================================================
//= OEditBaseModel
//==================================================================
class OEditBaseModel :	public OBoundControlModel
{
    sal_Int16					m_nLastReadVersion;

protected:
// [properties]			fuer all Editierfelder
    ::com::sun::star::uno::Any	m_aDefault;
    ::rtl::OUString				m_aDefaultText;				// default value
    sal_Bool					m_bEmptyIsNull : 1;			// empty string will be interepreted as NULL when committing
    sal_Bool					m_bFilterProposal : 1;		// use a list of possible value in filtermode
// [properties]

    sal_Int16	getLastReadVersion() const { return m_nLastReadVersion; }

public:
    DECLARE_DEFAULT_XTOR( OEditBaseModel );

    // XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    // XPropertyState
    virtual	::com::sun::star::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual	void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

protected:
    // new properties common to all edit models should be handled with the following two methods
    void SAL_CALL readCommonEditProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream);
    void SAL_CALL writeCommonEditProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream);
    void defaultCommonEditProperties();

    virtual sal_Int16 getPersistenceFlags() const;
        // derived classes may use this if they want this base class to write additinal version flags
        // (one of the PF_.... constants). After ::read they may ask for that flags with getLastReadVersion
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_EDITBASE_HXX_

