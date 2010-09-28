/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "pdfwriter_impl.hxx"

#include "cppuhelper/implbase1.hxx"

#include <rtl/digest.h>

using namespace vcl;
using namespace rtl;
using namespace com::sun::star;

/* a crutch to transport an rtlDigest safely though UNO API
   this is needed for the PDF export dialog, which otherwise would have to pass
   clear text passwords down till they can be used in PDFWriter. Unfortunately
   the MD5 sum of the password (which is needed to create the PDF encryption key)
   is not sufficient, since an rtl MD5 digest cannot be created in an arbitrary state
   which would be needed in PDFWriterImpl::computeEncryptionKey.
*/
class EncHashTransporter : public cppu::WeakImplHelper1 < com::sun::star::beans::XMaterialHolder >
{
    rtlDigest                   maUDigest;
    sal_IntPtr                  maID;
    std::vector< sal_uInt8 >    maOValue;

    static std::map< sal_IntPtr, EncHashTransporter* >      sTransporters;
public:
    EncHashTransporter()
    : maUDigest( rtl_digest_createMD5() )
    {
        maID = reinterpret_cast< sal_IntPtr >(this);
        while( sTransporters.find( maID ) != sTransporters.end() ) // paranoia mode
            maID++;
        sTransporters[ maID ] = this;
    }

    virtual ~EncHashTransporter()
    {
        sTransporters.erase( maID );
        if( maUDigest )
            rtl_digest_destroyMD5( maUDigest );
        OSL_TRACE( "EncHashTransporter freed\n" );
    }

    rtlDigest getUDigest() const { return maUDigest; };
    std::vector< sal_uInt8 >& getOValue() { return maOValue; }
    void invalidate()
    {
        if( maUDigest )
        {
            rtl_digest_destroyMD5( maUDigest );
            maUDigest = NULL;
        }
    }

    // XMaterialHolder
    virtual uno::Any SAL_CALL getMaterial() throw()
    {
        return uno::makeAny( sal_Int64(maID) );
    }

    static EncHashTransporter* getEncHashTransporter( const uno::Reference< beans::XMaterialHolder >& );

};

std::map< sal_IntPtr, EncHashTransporter* > EncHashTransporter::sTransporters;

EncHashTransporter* EncHashTransporter::getEncHashTransporter( const uno::Reference< beans::XMaterialHolder >& xRef )
{
    EncHashTransporter* pResult = NULL;
    if( xRef.is() )
    {
        uno::Any aMat( xRef->getMaterial() );
        sal_Int64 nMat = 0;
        if( aMat >>= nMat )
        {
            std::map< sal_IntPtr, EncHashTransporter* >::iterator it = sTransporters.find( static_cast<sal_IntPtr>(nMat) );
            if( it != sTransporters.end() )
                pResult = it->second;
        }
    }
    return pResult;
}

sal_Bool PDFWriterImpl::checkEncryptionBufferSize( register sal_Int32 newSize )
{
    if( m_nEncryptionBufferSize < newSize )
    {
        /* reallocate the buffer, the used function allocate as rtl_allocateMemory
        if the pointer parameter is NULL */
        m_pEncryptionBuffer = (sal_uInt8*)rtl_reallocateMemory( m_pEncryptionBuffer, newSize );
        if( m_pEncryptionBuffer )
            m_nEncryptionBufferSize = newSize;
        else
            m_nEncryptionBufferSize = 0;
    }
    return ( m_nEncryptionBufferSize != 0 );
}

void PDFWriterImpl::checkAndEnableStreamEncryption( register sal_Int32 nObject )
{
    if( m_aContext.Encryption.Encrypt() )
    {
        m_bEncryptThisStream = true;
        sal_Int32 i = m_nKeyLength;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)nObject;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 8 );
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 16 );
        //the other location of m_nEncryptionKey are already set to 0, our fixed generation number
        // do the MD5 hash
        sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
        // the i+2 to take into account the generation number, always zero
        rtl_digest_MD5( &m_aContext.Encryption.EncryptionKey[0], i+2, nMD5Sum, sizeof(nMD5Sum) );
        // initialize the RC4 with the key
        // key legth: see algoritm 3.1, step 4: (N+5) max 16
        rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum, m_nRC4KeyLength, NULL, 0 );
    }
}

void PDFWriterImpl::enableStringEncryption( register sal_Int32 nObject )
{
    if( m_aContext.Encryption.Encrypt() )
    {
        sal_Int32 i = m_nKeyLength;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)nObject;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 8 );
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 16 );
        //the other location of m_nEncryptionKey are already set to 0, our fixed generation number
        // do the MD5 hash
        sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
        // the i+2 to take into account the generation number, always zero
        rtl_digest_MD5( &m_aContext.Encryption.EncryptionKey[0], i+2, nMD5Sum, sizeof(nMD5Sum) );
        // initialize the RC4 with the key
        // key legth: see algoritm 3.1, step 4: (N+5) max 16
        rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum, m_nRC4KeyLength, NULL, 0 );
    }
}

/* init the encryption engine
1. init the document id, used both for building the document id and for building the encryption key(s)
2. build the encryption key following algorithms described in the PDF specification
 */
uno::Reference< beans::XMaterialHolder > PDFWriterImpl::initEncryption( const rtl::OUString& i_rOwnerPassword,
                                                                        const rtl::OUString& i_rUserPassword,
                                                                        bool b128Bit
                                                                        )
{
    uno::Reference< beans::XMaterialHolder > xResult;
    if( i_rOwnerPassword.getLength() || i_rUserPassword.getLength() )
    {
        EncHashTransporter* pTransporter = new EncHashTransporter;
        xResult = pTransporter;

        // get padded passwords
        sal_uInt8 aPadUPW[ENCRYPTED_PWD_SIZE], aPadOPW[ENCRYPTED_PWD_SIZE];
        padPassword( i_rOwnerPassword.getLength() ? i_rOwnerPassword : i_rUserPassword, aPadOPW );
        padPassword( i_rUserPassword, aPadUPW );
        sal_Int32 nKeyLength = SECUR_40BIT_KEY;
        if( b128Bit )
            nKeyLength = SECUR_128BIT_KEY;

        if( computeODictionaryValue( aPadOPW, aPadUPW, pTransporter->getOValue(), nKeyLength ) )
        {
            rtlDigest aDig = pTransporter->getUDigest();
            if( rtl_digest_updateMD5( aDig, aPadUPW, ENCRYPTED_PWD_SIZE ) != rtl_Digest_E_None )
                xResult.clear();
        }
        else
            xResult.clear();

        // trash temporary padded cleartext PWDs
        rtl_zeroMemory( aPadOPW, sizeof(aPadOPW) );
        rtl_zeroMemory( aPadUPW, sizeof(aPadUPW) );

    }
    return xResult;
}

bool PDFWriterImpl::prepareEncryption( const uno::Reference< beans::XMaterialHolder >& xEnc )
{
    bool bSuccess = false;
    EncHashTransporter* pTransporter = EncHashTransporter::getEncHashTransporter( xEnc );
    if( pTransporter )
    {
        sal_Int32 nKeyLength = 0, nRC4KeyLength = 0;
        sal_Int32 nAccessPermissions = computeAccessPermissions( m_aContext.Encryption, nKeyLength, nRC4KeyLength );
        m_aContext.Encryption.OValue = pTransporter->getOValue();
        bSuccess = computeUDictionaryValue( pTransporter, m_aContext.Encryption, nKeyLength, nAccessPermissions );
    }
    if( ! bSuccess )
    {
        m_aContext.Encryption.OValue.clear();
        m_aContext.Encryption.UValue.clear();
        m_aContext.Encryption.EncryptionKey.clear();
    }
    return bSuccess;
}

sal_Int32 PDFWriterImpl::computeAccessPermissions( const vcl::PDFWriter::PDFEncryptionProperties& i_rProperties,
                                                   sal_Int32& o_rKeyLength, sal_Int32& o_rRC4KeyLength )
{
    /*
    2) compute the access permissions, in numerical form

    the default value depends on the revision 2 (40 bit) or 3 (128 bit security):
    - for 40 bit security the unused bit must be set to 1, since they are not used
    - for 128 bit security the same bit must be preset to 0 and set later if needed
    according to the table 3.15, pdf v 1.4 */
    sal_Int32 nAccessPermissions = ( i_rProperties.Security128bit ) ? 0xfffff0c0 : 0xffffffc0 ;

    /* check permissions for 40 bit security case */
    nAccessPermissions |= ( i_rProperties.CanPrintTheDocument ) ?  1 << 2 : 0;
    nAccessPermissions |= ( i_rProperties.CanModifyTheContent ) ? 1 << 3 : 0;
    nAccessPermissions |= ( i_rProperties.CanCopyOrExtract ) ?   1 << 4 : 0;
    nAccessPermissions |= ( i_rProperties.CanAddOrModify ) ? 1 << 5 : 0;
    o_rKeyLength = SECUR_40BIT_KEY;
    o_rRC4KeyLength = SECUR_40BIT_KEY+5; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 5

    if( i_rProperties.Security128bit )
    {
        o_rKeyLength = SECUR_128BIT_KEY;
        o_rRC4KeyLength = 16; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 16, thus maximum
        // permitted value is 16
        nAccessPermissions |= ( i_rProperties.CanFillInteractive ) ?         1 << 8 : 0;
        nAccessPermissions |= ( i_rProperties.CanExtractForAccessibility ) ? 1 << 9 : 0;
        nAccessPermissions |= ( i_rProperties.CanAssemble ) ?                1 << 10 : 0;
        nAccessPermissions |= ( i_rProperties.CanPrintFull ) ?               1 << 11 : 0;
    }
    return nAccessPermissions;
}

/*************************************************************
begin i12626 methods

Implements Algorithm 3.2, step 1 only
*/
void PDFWriterImpl::padPassword( const rtl::OUString& i_rPassword, sal_uInt8* o_pPaddedPW )
{
    // get ansi-1252 version of the password string CHECKIT ! i12626
    rtl::OString aString( rtl::OUStringToOString( i_rPassword, RTL_TEXTENCODING_MS_1252 ) );

    //copy the string to the target
    sal_Int32 nToCopy = ( aString.getLength() < ENCRYPTED_PWD_SIZE ) ? aString.getLength() : ENCRYPTED_PWD_SIZE;
    sal_Int32 nCurrentChar;

    for( nCurrentChar = 0; nCurrentChar < nToCopy; nCurrentChar++ )
        o_pPaddedPW[nCurrentChar] = (sal_uInt8)( aString.getStr()[nCurrentChar] );

    //pad it with standard byte string
    sal_Int32 i,y;
    for( i = nCurrentChar, y = 0 ; i < ENCRYPTED_PWD_SIZE; i++, y++ )
        o_pPaddedPW[i] = s_nPadString[y];

    // trash memory of temporary clear text password
    rtl_zeroMemory( (sal_Char*)aString.getStr(), aString.getLength() );
}

/**********************************
Algorithm 3.2  Compute the encryption key used

step 1 should already be done before calling, the paThePaddedPassword parameter should contain
the padded password and must be 32 byte long, the encryption key is returned into the paEncryptionKey parameter,
it will be 16 byte long for 128 bit security; for 40 bit security only the first 5 bytes are used

TODO: in pdf ver 1.5 and 1.6 the step 6 is different, should be implemented. See spec.

*/
bool PDFWriterImpl::computeEncryptionKey( EncHashTransporter* i_pTransporter, vcl::PDFWriter::PDFEncryptionProperties& io_rProperties, sal_Int32 i_nAccessPermissions )
{
    bool bSuccess = true;
    sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];

    // transporter contains an MD5 digest with the padded user password already
    rtlDigest aDigest = i_pTransporter->getUDigest();
    rtlDigestError nError = rtl_Digest_E_None;
    if( aDigest )
    {
        //step 3
        if( ! io_rProperties.OValue.empty() )
            nError = rtl_digest_updateMD5( aDigest, &io_rProperties.OValue[0] , sal_Int32(io_rProperties.OValue.size()) );
        else
            bSuccess = false;
        //Step 4
        sal_uInt8 nPerm[4];

        nPerm[0] = (sal_uInt8)i_nAccessPermissions;
        nPerm[1] = (sal_uInt8)( i_nAccessPermissions >> 8 );
        nPerm[2] = (sal_uInt8)( i_nAccessPermissions >> 16 );
        nPerm[3] = (sal_uInt8)( i_nAccessPermissions >> 24 );

        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( aDigest, nPerm , sizeof( nPerm ) );

        //step 5, get the document ID, binary form
        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( aDigest, &io_rProperties.DocumentIdentifier[0], sal_Int32(io_rProperties.DocumentIdentifier.size()) );
        //get the digest
        if( nError == rtl_Digest_E_None )
        {
            rtl_digest_getMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );

            //step 6, only if 128 bit
            if( io_rProperties.Security128bit )
            {
                for( sal_Int32 i = 0; i < 50; i++ )
                {
                    nError = rtl_digest_updateMD5( aDigest, &nMD5Sum, sizeof( nMD5Sum ) );
                    if( nError != rtl_Digest_E_None )
                    {
                        bSuccess =  false;
                        break;
                    }
                    rtl_digest_getMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                }
            }
        }
    }
    else
        bSuccess = false;

    i_pTransporter->invalidate();

    //Step 7
    if( bSuccess )
    {
        io_rProperties.EncryptionKey.resize( MAXIMUM_RC4_KEY_LENGTH );
        for( sal_Int32 i = 0; i < MD5_DIGEST_SIZE; i++ )
            io_rProperties.EncryptionKey[i] = nMD5Sum[i];
    }
    else
        io_rProperties.EncryptionKey.clear();

    return bSuccess;
}

/**********************************
Algorithm 3.3  Compute the encryption dictionary /O value, save into the class data member
the step numbers down here correspond to the ones in PDF v.1.4 specfication
*/
bool PDFWriterImpl::computeODictionaryValue( const sal_uInt8* i_pPaddedOwnerPassword,
                                             const sal_uInt8* i_pPaddedUserPassword,
                                             std::vector< sal_uInt8 >& io_rOValue,
                                             sal_Int32 i_nKeyLength
                                             )
{
    bool bSuccess = true;

    io_rOValue.resize( ENCRYPTED_PWD_SIZE );

    rtlDigest aDigest = rtl_digest_createMD5();
    rtlCipher aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );
    if( aDigest && aCipher)
    {
        //step 1 already done, data is in i_pPaddedOwnerPassword
        //step 2

        rtlDigestError nError = rtl_digest_updateMD5( aDigest, i_pPaddedOwnerPassword, ENCRYPTED_PWD_SIZE );
        if( nError == rtl_Digest_E_None )
        {
            sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];

            rtl_digest_getMD5( aDigest, nMD5Sum, sizeof(nMD5Sum) );
//step 3, only if 128 bit
            if( i_nKeyLength == SECUR_128BIT_KEY )
            {
                sal_Int32 i;
                for( i = 0; i < 50; i++ )
                {
                    nError = rtl_digest_updateMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                    if( nError != rtl_Digest_E_None )
                    {
                        bSuccess = false;
                        break;
                    }
                    rtl_digest_getMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                }
            }
            //Step 4, the key is in nMD5Sum
            //step 5 already done, data is in i_pPaddedUserPassword
            //step 6
            rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                     nMD5Sum, i_nKeyLength , NULL, 0 );
            // encrypt the user password using the key set above
            rtl_cipher_encodeARCFOUR( aCipher, i_pPaddedUserPassword, ENCRYPTED_PWD_SIZE, // the data to be encrypted
                                      &io_rOValue[0], sal_Int32(io_rOValue.size()) ); //encrypted data
            //Step 7, only if 128 bit
            if( i_nKeyLength == SECUR_128BIT_KEY )
            {
                sal_uInt32 i, y;
                sal_uInt8 nLocalKey[ SECUR_128BIT_KEY ]; // 16 = 128 bit key

                for( i = 1; i <= 19; i++ ) // do it 19 times, start with 1
                {
                    for( y = 0; y < sizeof( nLocalKey ); y++ )
                        nLocalKey[y] = (sal_uInt8)( nMD5Sum[y] ^ i );

                    rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                            nLocalKey, SECUR_128BIT_KEY, NULL, 0 ); //destination data area, on init can be NULL
                    rtl_cipher_encodeARCFOUR( aCipher, &io_rOValue[0], sal_Int32(io_rOValue.size()), // the data to be encrypted
                                              &io_rOValue[0], sal_Int32(io_rOValue.size()) ); // encrypted data, can be the same as the input, encrypt "in place"
                    //step 8, store in class data member
                }
            }
        }
        else
            bSuccess = false;
    }
    else
        bSuccess = false;

    if( aDigest )
        rtl_digest_destroyMD5( aDigest );
    if( aCipher )
        rtl_cipher_destroyARCFOUR( aCipher );

    if( ! bSuccess )
        io_rOValue.clear();
    return bSuccess;
}

/**********************************
Algorithms 3.4 and 3.5  Compute the encryption dictionary /U value, save into the class data member, revision 2 (40 bit) or 3 (128 bit)
*/
bool PDFWriterImpl::computeUDictionaryValue( EncHashTransporter* i_pTransporter,
                                             vcl::PDFWriter::PDFEncryptionProperties& io_rProperties,
                                             sal_Int32 i_nKeyLength,
                                             sal_Int32 i_nAccessPermissions
                                             )
{
    bool bSuccess = true;

    io_rProperties.UValue.resize( ENCRYPTED_PWD_SIZE );

    rtlDigest aDigest = rtl_digest_createMD5();
    rtlCipher aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );
    if( aDigest && aCipher )
    {
        //step 1, common to both 3.4 and 3.5
        if( computeEncryptionKey( i_pTransporter, io_rProperties, i_nAccessPermissions ) )
        {
            // prepare encryption key for object
            for( sal_Int32 i = i_nKeyLength, y = 0; y < 5 ; y++ )
                io_rProperties.EncryptionKey[i++] = 0;

            if( io_rProperties.Security128bit == false )
            {
                //3.4
                //step 2 and 3
                rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                        &io_rProperties.EncryptionKey[0], 5 , // key and key length
                                        NULL, 0 ); //destination data area
                // encrypt the user password using the key set above, save for later use
                rtl_cipher_encodeARCFOUR( aCipher, s_nPadString, sizeof( s_nPadString ), // the data to be encrypted
                                          &io_rProperties.UValue[0], sal_Int32(io_rProperties.UValue.size()) ); //encrypted data, stored in class data member
            }
            else
            {
                //or 3.5, for 128 bit security
                //step6, initilize the last 16 bytes of the encrypted user password to 0
                for(sal_uInt32 i = MD5_DIGEST_SIZE; i < sal_uInt32(io_rProperties.UValue.size()); i++)
                    io_rProperties.UValue[i] = 0;
                //step 2
                rtlDigestError nError = rtl_digest_updateMD5( aDigest, s_nPadString, sizeof( s_nPadString ) );
                //step 3
                if( nError == rtl_Digest_E_None )
                    nError = rtl_digest_updateMD5( aDigest, &io_rProperties.DocumentIdentifier[0], sal_Int32(io_rProperties.DocumentIdentifier.size()) );
                else
                    bSuccess = false;

                sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
                rtl_digest_getMD5( aDigest, nMD5Sum, sizeof(nMD5Sum) );
                //Step 4
                rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                        &io_rProperties.EncryptionKey[0], SECUR_128BIT_KEY, NULL, 0 ); //destination data area
                rtl_cipher_encodeARCFOUR( aCipher, nMD5Sum, sizeof( nMD5Sum ), // the data to be encrypted
                                          &io_rProperties.UValue[0], sizeof( nMD5Sum ) ); //encrypted data, stored in class data member
                //step 5
                sal_uInt32 i, y;
                sal_uInt8 nLocalKey[SECUR_128BIT_KEY];

                for( i = 1; i <= 19; i++ ) // do it 19 times, start with 1
                {
                    for( y = 0; y < sizeof( nLocalKey ) ; y++ )
                        nLocalKey[y] = (sal_uInt8)( io_rProperties.EncryptionKey[y] ^ i );

                    rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                            nLocalKey, SECUR_128BIT_KEY, // key and key length
                                            NULL, 0 ); //destination data area, on init can be NULL
                    rtl_cipher_encodeARCFOUR( aCipher, &io_rProperties.UValue[0], SECUR_128BIT_KEY, // the data to be encrypted
                                              &io_rProperties.UValue[0], SECUR_128BIT_KEY ); // encrypted data, can be the same as the input, encrypt "in place"
                }
            }
        }
        else
            bSuccess = false;
    }
    else
        bSuccess = false;

    if( aDigest )
        rtl_digest_destroyMD5( aDigest );
    if( aCipher )
        rtl_cipher_destroyARCFOUR( aCipher );

    if( ! bSuccess )
        io_rProperties.UValue.clear();
    return bSuccess;
}

/* end i12626 methods */

