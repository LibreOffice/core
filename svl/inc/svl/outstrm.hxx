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

#ifndef SVTOOLS_OUTSTRM_HXX
#define SVTOOLS_OUTSTRM_HXX

#include "svl/svldllapi.h"
#include <com/sun/star/uno/Reference.h>
#include <tools/stream.hxx>

namespace com { namespace sun { namespace star { namespace io {
    class XOutputStream;
} } } }

//============================================================================
class SVL_DLLPUBLIC SvOutputStream: public SvStream
{
    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        m_xStream;

    SVL_DLLPRIVATE virtual sal_uLong GetData(void *, sal_uLong);

    SVL_DLLPRIVATE virtual sal_uLong PutData(void const * pData, sal_uLong nSize);

    SVL_DLLPRIVATE virtual sal_uLong SeekPos(sal_uLong);

    SVL_DLLPRIVATE virtual void FlushData();

    SVL_DLLPRIVATE virtual void SetSize(sal_uLong);

public:
    SvOutputStream(com::sun::star::uno::Reference<
                           com::sun::star::io::XOutputStream > const &
                       rTheStream);

    virtual ~SvOutputStream();

    virtual sal_uInt16 IsA() const;
};

#endif // SVTOOLS_OUTSTRM_HXX

