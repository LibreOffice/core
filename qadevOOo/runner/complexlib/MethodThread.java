package complexlib;

import java.io.PrintWriter;
import java.lang.reflect.Method;
import lib.StatusException;

/**
 * Invoke a method of a class in an own thread. Provide a method to end
 * the thread.
 */
public class MethodThread extends Thread {

    /** The method that should be executed **/
    private Method mTestMethod = null;
    /** The object that implements the method **/
    private Object mInvokeClass = null;
    /** A PrintWriter for debug Output **/
    private PrintWriter mLog = null;
    /** An Error String **/
    private String mErrMessage = null;
    /** Did an Exception happen? **/
    private boolean mExceptionHappened = false;

    /**
     * Constructor.
     * @param testMethod The method that will be invoked.
     * @param invokeClass The class where the method is invoked.
     * @param log The logging mechanism.
     */
    public MethodThread(Method testMethod, Object invokeClass, PrintWriter log) {
        mTestMethod = testMethod;
        mInvokeClass = invokeClass;
        mLog = log;
    }

    /**
     * Invoke the method.
     */
    public void run() {
        try {
            mTestMethod.invoke(mInvokeClass, null);
        }
        catch(IllegalAccessException e) {
            e.printStackTrace(mLog);
            mErrMessage = e.getMessage();
            mExceptionHappened = true;
        }
        catch(java.lang.reflect.InvocationTargetException e) {
            Throwable t = e.getTargetException();
            if (!(t instanceof StatusException)) {
                t.printStackTrace(mLog);
                mErrMessage = t.getMessage();
                mExceptionHappened = true;
            }
        }
    }

    /**
     * Get the error message
     * @return The error message.
     */
    public String getErrorMessage() {
        return mErrMessage;
    }

    /**
     * Is there an error message?
     * @return True, if an error did happen.
     */
    public boolean hasErrorMessage() {
        return mExceptionHappened;
    }

    /**
     * Stop the running method.
     */
    public void destroy() {
        try {
            interrupt();
        }
        catch(SecurityException e) {
            e.printStackTrace(mLog);
            mErrMessage = e.getMessage();
            mExceptionHappened = true;
        }
    }
}
