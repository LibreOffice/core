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



#ifndef OOX_XLS_NUMBERFORMATSBUFFER_HXX
#define OOX_XLS_NUMBERFORMATSBUFFER_HXX

#include <com/sun/star/lang/Locale.hpp>
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace util { class XNumberFormats; }
} } }

namespace oox { class PropertyMap; }

namespace oox {
namespace xls {

// ============================================================================

struct NumFmtModel
{
    ::com::sun::star::lang::Locale maLocale;
    ::rtl::OUString     maFmtCode;
    sal_Int16           mnPredefId;

    explicit            NumFmtModel();
};

// ----------------------------------------------------------------------------

/** Contains all API number format attributes. */
struct ApiNumFmtData
{
    sal_Int32           mnIndex;            /// API number format index.

    explicit            ApiNumFmtData();
};

// ----------------------------------------------------------------------------

/** Contains all data for a number format code. */
class NumberFormat : public WorkbookHelper
{
public:
    explicit            NumberFormat( const WorkbookHelper& rHelper );

    /** Sets the passed format code. */
    void                setFormatCode( const ::rtl::OUString& rFmtCode );
    /** Sets the passed format code, encoded in UTF-8. */
    void                setFormatCode(
                            const ::com::sun::star::lang::Locale& rLocale,
                            const sal_Char* pcFmtCode );
    /** Sets the passed predefined format code identifier. */
    void                setPredefinedId(
                            const ::com::sun::star::lang::Locale& rLocale,
                            sal_Int16 nPredefId );

    /** Final processing after import of all style settings. Returns the API format index. */
    sal_Int32           finalizeImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >& rxNumFmts,
                            const ::com::sun::star::lang::Locale& rFromLocale );

    /** Writes the number format to the passed property map. */
    void                writeToPropertyMap( PropertyMap& rPropMap ) const;

private:
    NumFmtModel         maModel;
    ApiNumFmtData       maApiData;
};

typedef ::boost::shared_ptr< NumberFormat > NumberFormatRef;

// ============================================================================

class NumberFormatsBuffer : public WorkbookHelper
{
public:
    explicit            NumberFormatsBuffer( const WorkbookHelper& rHelper );

    /** Inserts a new number format. */
    NumberFormatRef     createNumFmt( sal_Int32 nNumFmtId, const ::rtl::OUString& rFmtCode );

    /** Inserts a new number format code. */
    NumberFormatRef     importNumFmt( const AttributeList& rAttribs );
    /** Inserts a new number format code from a NUMFMT record. */
    void                importNumFmt( SequenceInputStream& rStrm );
    /** Inserts a new number format code from a FORMAT record. */
    void                importFormat( BiffInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Writes the specified number format to the passed property map. */
    void                writeToPropertyMap( PropertyMap& rPropMap, sal_Int32 nNumFmtId ) const;

private:
    /** Inserts built-in number formats for the current system language. */
    void                insertBuiltinFormats();

private:
    typedef RefMap< sal_Int32, NumberFormat > NumberFormatMap;

    NumberFormatMap     maNumFmts;          /// List of number formats.
    ::rtl::OUString     maLocaleStr;        /// Current office locale.
    sal_Int32           mnNextBiffIndex;    /// Format id counter for BIFF2-BIFF4.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
