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



#include <text.hxx>


namespace DOM
{
    CText::CText(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            NodeType const& reNodeType, xmlNodePtr const& rpNode)
        : CText_Base(rDocument, rMutex, reNodeType, rpNode)
    {
    }

    CText::CText(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNodePtr const pNode)
        : CText_Base(rDocument, rMutex, NodeType_TEXT_NODE, pNode)
    {
    }

    void CText::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        i_xHandler->characters(getData());
    }

    void CText::fastSaxify( Context& io_rContext )
    {
        if (io_rContext.mxCurrentHandler.is())
        {
            try
            {
                io_rContext.mxCurrentHandler->characters( getData() );
            }
            catch( Exception& )
            {}
        }
    }

    OUString SAL_CALL CText::getNodeName() throw (RuntimeException)
    {
        return OUString::createFromAscii("#text");
    }

    Reference< XText > SAL_CALL CText::splitText(sal_Int32 /*offset*/)
         throw (RuntimeException)
    {
        OSL_ENSURE(false, "CText::splitText: not implemented (#i113683#)");
        return Reference< XText >(this);
    }
}
