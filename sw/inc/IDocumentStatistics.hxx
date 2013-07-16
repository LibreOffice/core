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

 #ifndef IDOCUMENTSTATISTICS_HXX_INCLUDED
 #define IDOCUMENTSTATISTICS_HXX_INCLUDED

 struct SwDocStat;

 /** Document statistics information
 */
 class IDocumentStatistics
 {
 public:

    /** DocInfo has changed (notify via DocShell):
        make required fields update.
    */
    virtual void DocInfoChgd() = 0;

    /** Document - Statistics
    */
    /// Returns a reference to the existing document statistics
    virtual const SwDocStat &GetDocStat() const = 0;

    /**
      * Updates the document statistics if the document has been
      * modified and returns a reference to the result.
      * \param bCompleteAsync if true will return a partial result,
      * and potentially trigger a timeout to complete the work.
      * \param bFields if stat. fields should be updated
      */
    virtual const SwDocStat &GetUpdatedDocStat(bool bCompleteAsync, bool bFields) = 0;

    /// Set the document statistics
    virtual void SetDocStat(const SwDocStat& rStat) = 0;

    /**
      * Updates the internal document's statistics
      * \param bCompleteAsync if true it may do part of the
      * work and trigger a timeout to complete it.
      * \param bFields if stat. fields should be updated
      */
    virtual void UpdateDocStat(bool bCompleteAsync, bool bFields) = 0;

protected:
    virtual ~IDocumentStatistics() {};
 };

 #endif // IDOCUMENTSTATISTICS_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
