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



#include <rtl/ustring.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

// Get the demo.rdb servcie manager...
::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > CreateDemoServiceFactory();

// Ask user to show more signature details...
void QueryPrintSignatureDetails( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > );

// Query value from user.
int QuerySelectNumber( int nMin, int nMax );

// Ask to verify the signature
long QueryVerifySignature();

// Open In/Output Stream
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > OpenInputStream( const ::rtl::OUString& rStreamName );
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > OpenOutputStream( const ::rtl::OUString& rStreamName );

::rtl::OUString getSignatureInformations( const SignatureInformations& SignatureInformations, ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment );
::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > getCertificateFromEnvironment( ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment, sal_Bool nType);
