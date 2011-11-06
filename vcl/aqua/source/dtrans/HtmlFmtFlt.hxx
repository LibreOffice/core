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

#ifndef INCLUDED_HTMLFMTFLT_HXX
#define INCLUDED_HTMLFMTFLT_HXX

#include <com/sun/star/uno/Sequence.hxx>

/* Transform plain HTML into the format expected by MS Office.
 */
com::sun::star::uno::Sequence<sal_Int8> TextHtmlToHTMLFormat(com::sun::star::uno::Sequence<sal_Int8>& aTextHtml);

/* Transform the MS Office HTML format into plain HTML.
 */
com::sun::star::uno::Sequence<sal_Int8> HTMLFormatToTextHtml(const com::sun::star::uno::Sequence<sal_Int8>& aHTMLFormat);

/* Detects whether the given byte sequence contains the MS Office Html format.

   @returns True if the MS Office Html format will be detected False otherwise.
 */
bool isHTMLFormat (const com::sun::star::uno::Sequence<sal_Int8>& aHtmlSequence);

#endif
