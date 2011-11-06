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



#ifndef SC_FORMULAPARSERPOOL_HXX
#define SC_FORMULAPARSERPOOL_HXX

#include <hash_map>
#include <com/sun/star/sheet/XFormulaParser.hpp>

class ScDocument;

// ============================================================================

/** Stores the used instances of the FilterFormulaParser service
    implementations, mapped by the formula namespace they support. */
class ScFormulaParserPool
{
public:
    explicit            ScFormulaParserPool( const ScDocument& rDoc );
                        ~ScFormulaParserPool();

    /** Returns true, if a formula parser is registered for the passed namespace. */
    bool                hasFormulaParser( const ::rtl::OUString& rNamespace );

    /** Returns the formula parser that is registered for the passed namespace. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser >
                        getFormulaParser( const ::rtl::OUString& rNamespace );

private:
    typedef ::std::hash_map<
        ::rtl::OUString,
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser >,
        ::rtl::OUStringHash,
        ::std::equal_to< ::rtl::OUString > > ParserMap;

    const ScDocument&   mrDoc;
    ParserMap           maParsers;
};

// ============================================================================

#endif

