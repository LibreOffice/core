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

#include <filter.hxx>
#include <editutil.hxx>
#include <rtfimp.hxx>
#include <rtfparse.hxx>
#include <ftools.hxx>

ErrCode ScFormatFilterPluginImpl::ScImportRTF( SvStream &rStream, const OUString& rBaseURL, ScDocument *pDoc, ScRange& rRange )
{
    ScRTFImport aImp( pDoc, rRange );
    ErrCode nErr = aImp.Read( rStream, rBaseURL );
    ScRange aR = aImp.GetRange();
    rRange.aEnd = aR.aEnd;
    aImp.WriteToDocument();
    return nErr;
}

std::unique_ptr<ScEEAbsImport> ScFormatFilterPluginImpl::CreateRTFImport( ScDocument* pDoc, const ScRange& rRange )
{
    return std::make_unique<ScRTFImport>( pDoc, rRange );
}

ScRTFImport::ScRTFImport( ScDocument* pDocP, const ScRange& rRange ) :
    ScEEImport( pDocP, rRange )
{
    mpParser.reset(new ScRTFParser( mpEngine.get() ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
