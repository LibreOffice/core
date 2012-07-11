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

#ifndef ADC_TKP_HXX
#define ADC_TKP_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
class CharacterSource;
class TkpContext;
    // PARAMETRS



/** This is the interface for parser classes, which get a sequence of tokens from
    a text.

    Start() starts to parse the text from the given i_rSource.

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/

class TokenParser
{
  public:
    // LIFECYCLE
                        TokenParser();
    virtual             ~TokenParser() {}

    // OPERATIONS
    /** Start parsing a character source. Any previously parsed sources
        are discarded.
    */
    virtual void        Start(
                            CharacterSource &
                                            i_rSource );

    /// @return true, if there are more tokens to parse.
    bool                HasMore() const         { return bHasMore; }

  private:
    void                InitSource(
                            CharacterSource &
                                            i_rSource );

    virtual void        SetStartContext() = 0;
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext ) = 0;
    virtual TkpContext &
                        CurrentContext() = 0;
    // DATA
    CharacterSource *   pChars;
    bool                bHasMore;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
