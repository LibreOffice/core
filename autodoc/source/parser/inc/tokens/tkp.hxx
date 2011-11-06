/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    GetNextToken() returns a Token on the heap as long as there are
    still characters in the text left. This can be checked by
    HasMore().

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/
#if 0
/**
    Parsing can be interrupted for a different source by PushSource().
    The parsing before interruption is continued after PopSource().
*/
#endif // 0

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

    /** @short  Gets the next identifiable token out of the
        source code.
    */
    void                GetNextToken();

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


