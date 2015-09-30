/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_XMLOFF_INC_SCHXMLEXPORT_HXX
#define INCLUDED_XMLOFF_INC_SCHXMLEXPORT_HXX

#include <xmloff/SchXMLExportHelper.hxx>
#include "SchXMLAutoStylePoolP.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/prhdlfac.hxx>

namespace com { namespace sun { namespace star {
    namespace task {
        class XStatusIndicator;
    }
}}}

// export class for a complete chart document

class SchXMLExport : public SvXMLExport
{
private:
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    rtl::Reference<SchXMLAutoStylePoolP> maAutoStylePool;

    rtl::Reference<SchXMLExportHelper> maExportHelper;

protected:
    virtual sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) SAL_OVERRIDE;

    virtual void _ExportStyles( bool bUsed ) SAL_OVERRIDE;
    virtual void _ExportAutoStyles() SAL_OVERRIDE;
    virtual void _ExportMasterStyles() SAL_OVERRIDE;
    virtual void _ExportContent() SAL_OVERRIDE;

public:
    SchXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        OUString const & implementationName,
        SvXMLExportFlags nExportFlags = SvXMLExportFlags::ALL );
    virtual ~SchXMLExport();

    rtl::Reference< XMLPropertySetMapper > GetPropertySetMapper() const;
};

#endif // INCLUDED_XMLOFF_INC_SCHXMLEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
