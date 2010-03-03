/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mediadescriptor.hxx,v $
 * $Revision: 1.1 $
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

#ifndef OOX_HELPER_MEDIADESCRIPTOR_HXX
#define OOX_HELPER_MEDIADESCRIPTOR_HXX

#include <comphelper/mediadescriptor.hxx>
#include "oox/dllapi.h"

namespace oox {

// ============================================================================

class OOX_DLLPUBLIC MediaDescriptor : public ::comphelper::MediaDescriptor
{
public:
    explicit            MediaDescriptor();
    explicit            MediaDescriptor(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescSeq );

    /** Returns a value from the property 'ComponentData' of the media
        descriptor, if existing, otherwise an empty Any. */
    ::com::sun::star::uno::Any
                        getComponentDataEntry(
                            const ::rtl::OUString& rName ) const;

    /** Inserts a value into the property 'ComponentData' of the media
        descriptor. */
    void                setComponentDataEntry(
                            const ::rtl::OUString& rName,
                            const ::com::sun::star::uno::Any& rValue );
};

// ============================================================================

} // namespace oox

#endif

