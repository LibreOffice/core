
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.NotConnectedException;
import com.sun.star.io.BufferSizeExceededException;
import com.sun.star.io.IOException;

class TestStream implements XInputStream, XOutputStream {

    /**  An array of bytes to be returned when the XInputStream read() method
     * is called
     */
    private byte [] isBytes;

    /**
     *  A backup buffer where data sent to the XOutputStream write() method
     *  is placed, used for verification purposes.
     */
    private byte [] bytesBak;

    /**
     *  Maximum length of data returned by XInputStream read().
     */
    private static int inputLength = 256;

    /**
     *  Keeps track of how far into the XInputStream has been read.
     */
    private int currentLoc;


    /**
     *  Constructor.  Sets up the isBytes array.
     */
    public TestStream() {

        currentLoc = 0;

        isBytes = new byte [inputLength];
        for (int i=0; i < inputLength; i++) {
            isBytes[i] = (byte)i;
        }
    }

    /* XInputStream interfaces */

    public int readBytes(byte[][] aData, int toRead)
        throws NotConnectedException, BufferSizeExceededException,
        IOException {

        int checkSize = inputLength - currentLoc;

        if (checkSize <= 0) {
            return -1;
        } else if (toRead > checkSize) {
            System.arraycopy(isBytes, currentLoc, aData[0], 0, checkSize);
            currentLoc += checkSize;
            return checkSize;
        } else {
            System.arraycopy(isBytes, currentLoc, aData[0], 0, toRead);
            currentLoc += toRead;
            return toRead;
        }
    }

    public int readSomeBytes(byte[][] aData, int nMaxBytesToRead)
        throws NotConnectedException,
        BufferSizeExceededException, IOException {

        return(readBytes(aData, nMaxBytesToRead));
    }

    public void skipBytes(int nBytesToSkip) throws NotConnectedException,
        BufferSizeExceededException, IOException {

        currentLoc += nBytesToSkip;
    }

    public int available() throws NotConnectedException, IOException {
        int checkSize = inputLength - currentLoc;

        if (checkSize < 0) {
            return -1;
        } else {
            return checkSize;
        }
    }

    public void closeInput() throws NotConnectedException, IOException {

        // Set currentLoc to the max, so read calls will return -1.
        //
        currentLoc = inputLength + 1;
        System.out.println("Closed XInputStream.");
    }

    /* XOutputStream Interfaces */

    public void writeBytes(byte [] aData)
        throws NotConnectedException, BufferSizeExceededException,
        IOException {

        // Set backup array, used for verification purposes.
        //
        bytesBak = aData;

        System.out.println("Wrote out the following data to XOutputStream:");
        for (int i=0; i < aData.length; i++) {
            System.out.println("  <" + aData[i] + ">");
        }
    }

    public void flush()
        throws NotConnectedException, BufferSizeExceededException,
        IOException {

        System.out.println("Flushed XOutputStream.");
    }

    public void closeOutput()
        throws NotConnectedException, BufferSizeExceededException,
        IOException {

        System.out.println("Closed XOutputStream.");
    }

    /**
     *  Returns the last data passed into the write function, used for
     *  verification purposes.
     */
    public byte [] getBytesBak() {
        return bytesBak;
    }

    public static void main(String args[]) {

        System.out.println("\nInputStream Test:");
        System.out.println("Testing read(), bytes value 1-256:");

        TestStream ts = new TestStream();
        XInputStreamToInputStreamAdapter is =
            new XInputStreamToInputStreamAdapter(ts);
        int rc = 0, avail;
        boolean testStatus = true;

        int cnt = 0;
        do {
            try {
                rc    = is.read();
                avail = is.available();
                System.out.println("  Read value <" + rc +
                    ">, avail <" + avail + ">");

                if (cnt < inputLength && rc != cnt) {
                   System.out.println("Read wrong value <" + rc + ">, expecting <" + cnt + ">");
                   testStatus = false;
                }

                cnt++;
            } catch (Exception e) {
                System.out.println("Error reading from InputStream");
                System.out.println("Error msg: " + e.getMessage());
                testStatus = false;
            }
        } while (rc >= 0);
        try {
            is.close();
        } catch (Exception e) {
            System.out.println("Error closing InputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        System.out.println("\nInputStream Test:");
        System.out.println("Testing read(), bytes value 1-256 with skips of 3 bytes:");
        ts = new TestStream();
        is = new XInputStreamToInputStreamAdapter(ts);

        cnt = 0;
        do {
            try {
                rc    = is.read();
                avail = is.available();
                System.out.println("  Read value <" + rc +
                    ">, avail <" + avail + ">");
                is.skip(3);

                if (cnt < inputLength && rc != cnt) {
                   System.out.println("Read wrong value <" + rc + ">, expecting <" + cnt + ">");
                   testStatus = false;
                }

                cnt += 4;
            } catch (Exception e) {
                System.out.println("Error reading from InputStream");
                System.out.println("Error msg: " + e.getMessage());
                testStatus = false;
            }
        } while (rc >= 0);
        try {
            is.close();
        } catch (Exception e) {
            System.out.println("Error closing InputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        System.out.println("\nInputStream Test:");
        System.out.println("Testing read() in chunks of 5 bytes.");
        byte [] bi1 = new byte [5];
        ts = new TestStream();
        is = new XInputStreamToInputStreamAdapter(ts);

        cnt = 0;
        do {
            try {
                rc    = is.read(bi1);
                avail = is.available();
                System.out.print("Read value <");
                for (int i=0; i < bi1.length; i++) {

                    if (i == (bi1.length - 1)) {
                        System.out.print((int)bi1[i]);
                    } else {
                        System.out.print((int)bi1[i] + ",");
                    }

                    if ((cnt) < inputLength && bi1[i] != (byte)cnt) {
                       System.out.println("\nRead wrong value <" + (int)bi1[i] + ">, expecting <" +
                           (cnt) + ">");
                       testStatus = false;
                    }
                    cnt++;
                }
                System.out.print("> read rc <" + rc +
                    ">, avail <" + avail + ">\n");
            } catch (Exception e) {
                System.out.println("Error reading from InputStream");
                System.out.println("Error msg: " + e.getMessage());
                testStatus = false;
            }
        } while (rc >= 0);
        try {
            is.close();
        } catch (Exception e) {
            System.out.println("Error closing InputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        System.out.println("\nInputStream Test:");
        System.out.println("Testing read() in chunks of 6 bytes, offset=2, length=3.");
        byte [] bi2 = new byte [6];
        ts = new TestStream();
        is = new XInputStreamToInputStreamAdapter(ts);

        cnt = 0;
        int offset = 2;
        int length = 3;
        do {
            try {
                rc = is.read(bi2, offset, length);
                avail = is.available();
                System.out.print("Read value <");
                for (int i=0; i < bi2.length; i++) {
                    if (i == (bi2.length - 1)) {
                        System.out.print((int)bi2[i]);
                    } else {
                        System.out.print((int)bi2[i] + ",");
                    }

                    if (cnt < inputLength) {
                        if (i < offset || i >= (offset + length)) {
                            // Check values that should stay 0
                            //
                            if ((int)bi2[i] != 0) {
                                System.out.println("\nRead wrong value <" +
                                    (int)bi2[i] + ">, expecting <0>");
                                testStatus = false;
                            }
                        } else if (bi2[i] != (byte)cnt) {
                            // Check actually read values.
                            //
                            System.out.println("\nRead wrong value <" +
                                (int)bi2[i] + ">, expecting <" + cnt + ">");
                            testStatus = false;
                        } else {
                            cnt++;
                        }
                    }
                }
                System.out.print("> read rc <" + rc +
                    ">, avail <" + avail + ">\n");
            } catch (Exception e) {
                System.out.println("Error reading from InputStream");
                System.out.println("Error msg: " + e.getMessage());
                testStatus = false;
            }
        } while (rc >= 0);
        try {
            is.close();
        } catch (Exception e) {
            System.out.println("Error closing InputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        System.out.println("\nOutputStream Test:");
        System.out.println("Testing write() and flush()");

        ts = new TestStream();
        XOutputStreamToOutputStreamAdapter os =
            new XOutputStreamToOutputStreamAdapter(ts);

        for (int i=0; i < 5; i++) {
            try {
                os.write((byte)i);
                byte [] testBytes = ts.getBytesBak();

                if (testBytes[0] != i) {
                    System.out.println("Wrote wrong value <" + testBytes[0] + ">, expecting <i>");
                    testStatus = false;
                }
                os.flush();
            } catch (Exception e) {
                System.out.println("Error writing to OutputStream");
                System.out.println("Error msg: " + e.getMessage());
                testStatus = false;
            }
        }
        try {
            os.close();
        } catch (Exception e) {
            System.out.println("Error closing OutputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        System.out.println("\nOutputStream Test:");
        System.out.println("Testing write() with a chunk of 5 bytes");

        ts = new TestStream();
        os = new XOutputStreamToOutputStreamAdapter(ts);

        byte [] bo1 = new byte [5];
        for (int i=0; i < bo1.length; i++) {
           bo1[i] = (byte)i;
        }

        try {
            os.write(bo1);
            byte [] testBytes = ts.getBytesBak();

            for (int i=0; i < bo1.length; i++) {
                if (testBytes[i] != bo1[i]) {
                    System.out.println("Wrote wrong value <" + testBytes[i] + ">, expecting <" + bo1[i] + ">");
                    testStatus = false;
                }
            }
            os.flush();
        } catch (Exception e) {
            System.out.println("Error writing to OutputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }
        try {
            os.close();
        } catch (Exception e) {
            System.out.println("Error closing OutputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        System.out.println("\nOutputStream Test:");
        System.out.println("Testing write() with a chunk of 6 bytes, offset=2, length=3.");

        ts = new TestStream();
        os = new XOutputStreamToOutputStreamAdapter(ts);

        byte [] bo2 = new byte [6];
        for (int i=0; i < bo2.length; i++) {
           bo2[i] = (byte)i;
        }

        offset = 2;
        length = 3;
        try {
            os.write(bo2, offset, length);
            byte [] testBytes = ts.getBytesBak();

            for (int i=0; i < bo2.length; i++) {
                if ((i >= offset && i < (offset + length)) && testBytes[i-offset] != bo2[i]) {
                    System.out.println("Wrote wrong value <" + testBytes[i-offset] + ">, expecting <" + bo2[i] + ">");
                    testStatus = false;
                }
            }
            os.flush();
        } catch (Exception e) {
            System.out.println("Error writing to OutputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }
        try {
            os.close();
        } catch (Exception e) {
            System.out.println("Error closing OutputStream");
            System.out.println("Error msg: " + e.getMessage());
            testStatus = false;
        }

        if (testStatus == true) {
           System.out.println("Test passed...\n");
        } else {
           System.out.println("Test failed...\n");
        }

        if (testStatus == true) {
           System.out.println("\nAll tests passed...\n");
           System.exit(0);
        } else {
           System.out.println("\nSome tests failed...\n");
           System.exit(-1);
        }
    }
}
