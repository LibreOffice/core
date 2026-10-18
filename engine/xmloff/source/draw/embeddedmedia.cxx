/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include "embeddedmedia.hxx"

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <xmloff/xmlexp.hxx>

using namespace ::cpo::uno;
using namespace ::com::sun::star;

namespace xmloff
{
namespace
{
/** Split a path inside a package into its first segment and the rest. */
bool splitPath(OUString const& rPath, OUString& rDir, OUString& rRest)
{
    const sal_Int32 nIndex(rPath.indexOf(u'/'));
    if (nIndex < 0 || nIndex >= rPath.getLength())
    {
        rDir = OUString();
        rRest = rPath;
        return true;
    }
    if (nIndex == 0 || nIndex == rPath.getLength() - 1)
    {
        // the path must not start or end with '/'
        return false;
    }
    rDir = rPath.copy(0, nIndex);
    rRest = rPath.copy(nIndex + 1);
    return true;
}

/** Copy the element at rPath, with its media type, from xSource to xTarget. */
void copyElement(Reference<embed::XStorage> const& xSource,
                 Reference<embed::XStorage> const& xTarget, OUString const& rPath)
{
    OUString aDir;
    OUString aRest;
    if (!splitPath(rPath, aDir, aRest))
        throw RuntimeException();

    if (aDir.isEmpty())
        xSource->copyElementTo(rPath, xTarget, rPath);
    else
    {
        Reference<embed::XStorage> const xSubSource(
            xSource->openStorageElement(aDir, embed::ElementModes::READ));
        Reference<embed::XStorage> const xSubTarget(
            xTarget->openStorageElement(aDir, embed::ElementModes::WRITE));
        copyElement(xSubSource, xSubTarget, aRest);
    }
    Reference<embed::XTransactedObject> const xTransaction(xTarget, UNO_QUERY);
    if (xTransaction.is())
        xTransaction->commit();
}
}

OUString StoreEmbeddedMediaAndGetHref(SvXMLExport& rExport, OUString const& rURL)
{
    OUString aPath;
    if (!rURL.startsWithIgnoreAsciiCase("vnd.sun.star.Package:", &aPath))
        return rExport.GetRelativeReference(rURL);

    try
    {
        Reference<document::XStorageBasedDocument> const xSBD(rExport.GetModel(),
                                                              UNO_QUERY_THROW);
        Reference<embed::XStorage> const xSource(xSBD->getDocumentStorage(), UNO_SET_THROW);
        Reference<embed::XStorage> const xTarget(rExport.GetTargetStorage(), UNO_SET_THROW);

        copyElement(xSource, xTarget, aPath);

        return aPath;
    }
    catch (Exception const&)
    {
        TOOLS_INFO_EXCEPTION("xmloff", "exception while storing embedded media");
    }
    return OUString();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
