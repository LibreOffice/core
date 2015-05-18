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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_CHARTSHEETFRAGMENT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_CHARTSHEETFRAGMENT_HXX

#include "excelhandlers.hxx"

namespace oox {
namespace xls {

class ChartsheetFragment : public WorksheetFragmentBase
{
public:
    explicit            ChartsheetFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onCharacters( const OUString& rChars ) SAL_OVERRIDE;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) SAL_OVERRIDE;

    virtual const ::oox::core::RecordInfo* getRecordInfos() const SAL_OVERRIDE;
    virtual void        initializeImport() SAL_OVERRIDE;
    virtual void        finalizeImport() SAL_OVERRIDE;

private:
    /** Imports the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( SequenceInputStream& rStrm );
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
