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

using System;
using System.Reflection;
using System.Diagnostics;
using uno;


using unoidl.test.cliure.climaker;
//using unoidl.com.sun.star.uno;
using ucss=unoidl.com.sun.star;

/** This class is for testing the generated code in the uno services
 */

public class Context: ucss.uno.XComponentContext
{
    public enum test_kind {
        NORMAL,
        NO_FACTORY,
        TEST_EXCEPTION,
        CREATION_FAILED
    }
    
    public Context(test_kind k, params object[] args)
    {
        kind = k;
		factory = new Factory(k, args);
    }
        
    public ucss.lang.XMultiComponentFactory getServiceManager()
    {
        if (kind == test_kind.NO_FACTORY)
            return null;
        return factory;
    }

    public Any  getValueByName(string Name)
    {
        if (kind == test_kind.NORMAL)
        {
            if (Name == "/singletons/unoidl.test.cliure.climaker.S4")
            {
                Component c = new Component(this);
                return new Any(typeof(object), c);
            }
        }
        else if (kind == test_kind.CREATION_FAILED)
        {
            return new Any();
        }
        return new Any();
    }

    class Factory: ucss.lang.XMultiComponentFactory
    {
        public Factory(Context.test_kind k, params object[] args) {
            kind2 = k;
            if (k == Context.test_kind.TEST_EXCEPTION)
                exception = (ucss.uno.Exception) args[0];
        }
        public object  createInstanceWithArgumentsAndContext(
            string ServiceSpecifier,
            uno.Any[] Arguments,
            unoidl.com.sun.star.uno.XComponentContext Context) {
            switch (kind2) {
            case test_kind.NORMAL:
                return new Component(Context, Arguments);
            case test_kind.CREATION_FAILED :
                return null;
            case test_kind.TEST_EXCEPTION:
                throw exception;
            default:
                throw new Exception("Factory not properly initialized");
            }
        }
        public object  createInstanceWithContext(
            string aServiceSpecifier,
            unoidl.com.sun.star.uno.XComponentContext Context) {
            switch (kind2) {
            case test_kind.NORMAL:
                return  new Component(Context);
            case test_kind.CREATION_FAILED:
                return null;
            case test_kind.TEST_EXCEPTION:
                throw exception;
            default:
                throw new Exception("Factory not properly initialized");
            }
        }

        public string[]  getAvailableServiceNames()
        {
            return new string[]{};
        }
        ucss.uno.Exception exception;
        test_kind kind2;
    }


    Factory factory;
    test_kind kind;
}


public class Logger
{
    String m_sFunction;
    int m_nErrors;
    public Logger() {
    }
    
	public String Function
	{
		set
		{
			m_sFunction = value;
		}
		get
		{
			return m_sFunction;
		}
	}
    
    public void assure(bool b) {
        if (b == false)
        {
            Console.WriteLine(m_sFunction + " failed!");
            m_nErrors++;
        }
    }

    public void printStatus() {
        Console.WriteLine("\n=====================================");

        
        String msg;
        if (m_nErrors > 0)
            msg = "Test failed! " + m_nErrors.ToString() + " Errors.";
        else
            msg = "Test succeeded!";

        Console.WriteLine(msg + "\n=====================================");
    }

    public int Errors
    {
        get
            {
                return m_nErrors;
            }
    }
}

public sealed class Test
{
    public static int Main(String[] args)
    {
        
//        System.Diagnostics.Debugger.Launch();
		try
		{
			Logger log = new Logger();
			Test t = new Test();
			t.testEnum1(log);
			t.testEnum2(log);
			t.testPolyStruct(log);
			t.testEmptyStruct2(log);
			t.testFullStruct2(log);
			t.testS1(log);
			t.testSingletons(log);
			t.testAttributes(log);
			t.testPolyStructAttributes(log);
			t.testPolymorphicType(log);
            t.testInterface(log);
            t.testAny(log);
			log.printStatus();
			if (log.Errors == 0)
				return 0;
			return -1;
		}
		catch(Exception e)
		{
			Console.Write(e.Message);
		}
		return -1;

	}


    public void testEnum1(Logger l) {
        l.Function = "testEnum1";
        l.assure(((int)Enum1.VALUE1) == -100);
        l.assure(((int)Enum1.VALUE2) == 100);
    }

    public void testEnum2(Logger l) {
        l.Function = "testEnum2";
        l.assure( ((int) Enum2.VALUE0) == 0);
        l.assure( ((int) Enum2.VALUE1) == 1);
        l.assure( ((int) Enum2.VALUE2) == 2);
        l.assure( ((int) Enum2.VALUE4) == 4);
    }

    public void testPolyStruct(Logger l) {
        l.Function = "testPolyStruct";
        PolyStruct s = new PolyStruct();
        l.assure(s.member1 == null);
        l.assure(s.member2 == 0);
        s = new PolyStruct("ABC", 5);
        l.assure(s.member1.Equals("ABC"));
        l.assure(s.member2 == 5);
    }

    public void testEmptyStruct2(Logger l) {
        l.Function = "testEmptyStruct2";
        Struct2 s = new Struct2();
        l.assure(s.p1 == false);
        l.assure(s.p2 == 0);
        l.assure(s.p3 == 0);
        l.assure(s.p4 == 0);
        l.assure(s.p5 == 0);
        l.assure(s.p6 == 0);
        l.assure(s.p7 == 0);
        l.assure(s.p8 == 0);
        l.assure(s.p9 == 0.0f);
        l.assure(s.p10 == 0.0);
        l.assure(s.p11 == '\u0000');
        l.assure(s.p12.Equals(""));
        l.assure(s.p13.Equals(typeof(void)));
        l.assure(s.p14.Equals(Any.VOID));
        l.assure(s.p15 == Enum2.VALUE0);
        l.assure(s.p16.member1 == 0);
        l.assure(s.p17 == null);
        l.assure(s.p18 == null);
        l.assure(s.t1 == false);
        l.assure(s.t2 == 0);
        l.assure(s.t3 == 0);
        l.assure(s.t4 == 0);
        l.assure(s.t5 == 0);
        l.assure(s.t6 == 0);
        l.assure(s.t7 == 0);
        l.assure(s.t8 == 0);
        l.assure(s.t9 == 0.0f);
        l.assure(s.t10 == 0.0);
        l.assure(s.t11 == '\u0000');
        l.assure(s.t12.Equals(""));
        l.assure(s.t13.Equals(typeof(void)));
        l.assure(s.t14.Equals(Any.VOID));
        l.assure(s.t15 == Enum2.VALUE0);
        l.assure(s.t16.member1 == 0);
        l.assure(s.t17 == null);
        l.assure(s.t18 == null);
        l.assure(s.a1.Length == 0);
        l.assure(s.a2.Length == 0);
        l.assure(s.a3.Length == 0);
        l.assure(s.a4.Length == 0);
        l.assure(s.a5.Length == 0);
        l.assure(s.a6.Length == 0);
        l.assure(s.a7.Length == 0);
        l.assure(s.a8.Length == 0);
        l.assure(s.a9.Length == 0);
        l.assure(s.a10.Length == 0);
        l.assure(s.a11.Length == 0);
        l.assure(s.a12.Length == 0);
        l.assure(s.a13.Length == 0);
        l.assure(s.a14.Length == 0);
        l.assure(s.a15.Length == 0);
        l.assure(s.a16.Length == 0);
        l.assure(s.a17.Length == 0);
        l.assure(s.a18.Length == 0);
        l.assure(s.aa1.Length == 0);
        l.assure(s.aa2.Length == 0);
        l.assure(s.aa3.Length == 0);
        l.assure(s.aa4.Length == 0);
        l.assure(s.aa5.Length == 0);
        l.assure(s.aa6.Length == 0);
        l.assure(s.aa7.Length == 0);
        l.assure(s.aa8.Length == 0);
        l.assure(s.aa9.Length == 0);
        l.assure(s.aa10.Length == 0);
        l.assure(s.aa11.Length == 0);
        l.assure(s.aa12.Length == 0);
        l.assure(s.aa13.Length == 0);
        l.assure(s.aa14.Length == 0);
        l.assure(s.aa15.Length == 0);
        l.assure(s.aa16.Length == 0);
        l.assure(s.aa17.Length == 0);
        l.assure(s.aa18.Length == 0);
        l.assure(s.at1.Length == 0);
        l.assure(s.at2.Length == 0);
        l.assure(s.at3.Length == 0);
        l.assure(s.at4.Length == 0);
        l.assure(s.at5.Length == 0);
        l.assure(s.at6.Length == 0);
        l.assure(s.at7.Length == 0);
        l.assure(s.at8.Length == 0);
        l.assure(s.at9.Length == 0);
        l.assure(s.at10.Length == 0);
        l.assure(s.at11.Length == 0);
        l.assure(s.at12.Length == 0);
        l.assure(s.at13.Length == 0);
        l.assure(s.at14.Length == 0);
        l.assure(s.at15.Length == 0);
        l.assure(s.at16.Length == 0);
        l.assure(s.at17.Length == 0);
        l.assure(s.at18.Length == 0);
    }
 
    public void testFullStruct2(Logger l) {
        //TODO:
        Struct2 s = new Struct2(
            true, (byte) 1, (short) 2, (ushort) 3, 4, 5, 6L, 7L, 0.8f, 0.9d, 'A',
            "BCD", typeof(ulong), new Any(22), Enum2.VALUE4,
            new Struct1(1), null, null, false, (byte) 0, (short) 0, (ushort) 0,
            0, 0, 0L, 0L, 0.0f, 0.0, '\u0000', "", typeof(void), Any.VOID,
            Enum2.VALUE0, new Struct1(), null, null,
            new bool[] { false, true }, new byte[] { (byte) 1, (byte) 2 },
            new short[0], new ushort[0], new int[0], new uint[0],
            new long[0], new ulong[0], new float[0], new double[0], new char[0],
            new String[0], new Type[0], new Any[0], new Enum2[0],
            new Struct1[] { new Struct1(1), new Struct1(2) }, new Object[0],
            new ucss.uno.XNamingService[0], new bool[0][], new byte[0][],
            new short[0][], new ushort[0][], new int[0][], new uint[0][],
            new long[0][], new ulong[0][], new float[0][], new double[0][],
            new char[0][], new String[0][], new Type[0][], new Any[0][],
            new Enum2[0][], new Struct1[0][], new Object[0][],
            new ucss.uno.XNamingService[0][], new bool[0][], new byte[0][],
            new short[0][], new ushort[0][], new int[0][], new uint[0][],
            new long[0][], new ulong[0][], new float[0][], new double[0][],
            new char[0][], new String[0][], new Type[0][], new Any[0][],
            new Enum2[0][], new Struct1[0][], new Object[0][],
            new ucss.uno.XNamingService[0][]);
        l.assure(s.p1 == true);
        l.assure(s.p2 == 1);
        l.assure(s.p3 == 2);
        l.assure(s.p4 == 3);
        l.assure(s.p5 == 4);
        l.assure(s.p6 == 5);
        l.assure(s.p7 == 6);
        l.assure(s.p8 == 7);
        l.assure(s.p9 == 0.8f);
        l.assure(s.p10 == 0.9);
        l.assure(s.p11 == 'A');
        l.assure(s.p12.Equals("BCD"));
        l.assure(s.p13.Equals(typeof(ulong)));
        l.assure(s.p14.Equals(new Any(22)));
        l.assure(s.p15 == Enum2.VALUE4);
        l.assure(s.p16.member1 == 1);
        l.assure(s.p17 == null);
        l.assure(s.p18 == null);
        l.assure(s.t1 == false);
        l.assure(s.t2 == 0);
        l.assure(s.t3 == 0);
        l.assure(s.t4 == 0);
        l.assure(s.t5 == 0);
        l.assure(s.t6 == 0);
        l.assure(s.t7 == 0);
        l.assure(s.t8 == 0);
        l.assure(s.t9 == 0.0f);
        l.assure(s.t10 == 0.0);
        l.assure(s.t11 == '\u0000');
        l.assure(s.t12.Equals(""));
        l.assure(s.t13.Equals(typeof(void)));
        l.assure(s.t14.Equals(Any.VOID));
        l.assure(s.t15 == Enum2.VALUE0);
        l.assure(s.t16.member1 == 0);
        l.assure(s.t17 == null);
        l.assure(s.t18 == null);
        l.assure(s.a1.Length == 2);
        l.assure(s.a1[0] == false);
        l.assure(s.a1[1] == true);
        l.assure(s.a2.Length == 2);
        l.assure(s.a2[0] == 1);
        l.assure(s.a2[1] == 2);
        l.assure(s.a3.Length == 0);
        l.assure(s.a4.Length == 0);
        l.assure(s.a5.Length == 0);
        l.assure(s.a6.Length == 0);
        l.assure(s.a7.Length == 0);
        l.assure(s.a8.Length == 0);
        l.assure(s.a9.Length == 0);
        l.assure(s.a10.Length == 0);
        l.assure(s.a11.Length == 0);
        l.assure(s.a12.Length == 0);
        l.assure(s.a13.Length == 0);
        l.assure(s.a14.Length == 0);
        l.assure(s.a15.Length == 0);
        l.assure(s.a16.Length == 2);
        l.assure(s.a16[0].member1 == 1);
        l.assure(s.a16[1].member1 == 2);
        l.assure(s.a17.Length == 0);
        l.assure(s.a18.Length == 0);
        l.assure(s.aa1.Length == 0);
        l.assure(s.aa2.Length == 0);
        l.assure(s.aa3.Length == 0);
        l.assure(s.aa4.Length == 0);
        l.assure(s.aa5.Length == 0);
        l.assure(s.aa6.Length == 0);
        l.assure(s.aa7.Length == 0);
        l.assure(s.aa8.Length == 0);
        l.assure(s.aa9.Length == 0);
        l.assure(s.aa10.Length == 0);
        l.assure(s.aa11.Length == 0);
        l.assure(s.aa12.Length == 0);
        l.assure(s.aa13.Length == 0);
        l.assure(s.aa14.Length == 0);
        l.assure(s.aa15.Length == 0);
        l.assure(s.aa16.Length == 0);
        l.assure(s.aa17.Length == 0);
        l.assure(s.aa18.Length == 0);
        l.assure(s.at1.Length == 0);
        l.assure(s.at2.Length == 0);
        l.assure(s.at3.Length == 0);
        l.assure(s.at4.Length == 0);
        l.assure(s.at5.Length == 0);
        l.assure(s.at6.Length == 0);
        l.assure(s.at7.Length == 0);
        l.assure(s.at8.Length == 0);
        l.assure(s.at9.Length == 0);
        l.assure(s.at10.Length == 0);
        l.assure(s.at11.Length == 0);
        l.assure(s.at12.Length == 0);
        l.assure(s.at13.Length == 0);
        l.assure(s.at14.Length == 0);
        l.assure(s.at15.Length == 0);
        l.assure(s.at16.Length == 0);
        l.assure(s.at17.Length == 0);
        l.assure(s.at18.Length == 0);
    }

    public void testS1(Logger l) {
        l.Function = "testS1";
        object obj = new Object();
        ucss.uno.RuntimeException excRuntime =
            new ucss.uno.RuntimeException("RuntimeException", obj);
        ucss.uno.Exception excException =
            new ucss.uno.Exception("Exception", obj);
        ucss.lang.IllegalAccessException excIllegalAccess =
            new ucss.lang.IllegalAccessException("IllegalAccessException", obj);
        ucss.uno.DeploymentException excDeployment =
            new ucss.uno.DeploymentException("DeploymentException", obj);
        ucss.lang.InvalidListenerException excInvalidListener =
            new ucss.lang.InvalidListenerException("ListenerException", obj);
        
        /* create1 does not specify exceptions. Therefore RuntimeExceptions
           fly through and other exceptions cause a DeploymentException.
        */            
        try {
            S1.create1(new Context(Context.test_kind.TEST_EXCEPTION, excRuntime));
        } catch (ucss.uno.RuntimeException e) {
            l.assure(e.Message == excRuntime.Message
                     && e.Context == obj);
        } catch (System.Exception) {
            l.assure(false);
        }

        Context c = new Context(Context.test_kind.TEST_EXCEPTION, excException); 
        try {
            S1.create1(c);
        } catch (ucss.uno.DeploymentException e) {
            //The message of the original exception should be contained
            // in the Deploymentexception
            l.assure(e.Message.IndexOf(excException.Message) != -1 && e.Context == c);
        } catch (System.Exception) {
            l.assure(false);
        }

        /* create2 specifies many exceptions, including RuntimeException and Exception.
           Because Exception is specified all exceptions are allowed, hence all thrown
           exceptions fly through.
         */
        try {
            S1.create2(new Context(Context.test_kind.TEST_EXCEPTION, excRuntime));
        } catch (ucss.uno.RuntimeException e) {
            l.assure(e.Message == excRuntime.Message
                     && e.Context == obj);
        } catch (System.Exception) {
            l.assure(false);
        }

        try {
            S1.create2(new Context(Context.test_kind.TEST_EXCEPTION, excIllegalAccess));
        } catch (ucss.lang.IllegalAccessException e) {
            l.assure(e.Message == excIllegalAccess.Message
                     && e.Context == obj);
        } catch (System.Exception) {
            l.assure(false);
        }

        try {
            S1.create2(new Context(Context.test_kind.TEST_EXCEPTION, excException));
        } catch (ucss.uno.Exception e) {
            l.assure(e.Message == excException.Message
                     && e.Context == obj);
        } catch (System.Exception) {
            l.assure(false);
        }

        /* create3 specifies exceptions but no com.sun.star.uno.Exception. RuntimeException
           and derived fly through. Other specified exceptions are rethrown and all other
           exceptions cause a DeploymentException.
        */
        try {
            S1.create3(new Context(Context.test_kind.TEST_EXCEPTION, excDeployment),
                       new Any[]{});
        } catch (ucss.uno.DeploymentException e) {
            l.assure(e.Message == excDeployment.Message
                     && e.Context == obj);
        } catch (System.Exception) {
            l.assure(false);
        }

        try {
            S1.create3(new Context(Context.test_kind.TEST_EXCEPTION, excIllegalAccess),
                       new Any[0]);
        } catch (ucss.lang.IllegalAccessException e) {
            l.assure(e.Message == excIllegalAccess.Message
                     && e.Context == obj);
        } catch (System.Exception) {
            l.assure(false);
        }

        c = new Context(Context.test_kind.TEST_EXCEPTION, excInvalidListener);
        try {
            S1.create3(c, new Any[0]);
        } catch (ucss.uno.DeploymentException e) {
            l.assure(e.Message.IndexOf(excInvalidListener.Message) != -1
                     && e.Context == c);
        } catch (System.Exception) {
            l.assure(false);
        }

        /* test the case when the context cannot provide a service manager.
         */
        try {
            S1.create2(new Context(Context.test_kind.NO_FACTORY));
        } catch (ucss.uno.DeploymentException e) {
            l.assure(e.Message.Length > 0);
        } catch (System.Exception) {
            l.assure(false);
        }

        /* When the service manager returns a null pointer then a DeploymentException
         * is to be thrown.
         */
        try {
            S1.create2(new Context(Context.test_kind.CREATION_FAILED));
        } catch (ucss.uno.DeploymentException e) {
            l.assure(e.Message.Length > 0);
        } catch (System.Exception) {
            l.assure(false);
        }


        /** Test creation of components and if the passing of parameters works.
         */
        c = new Context(Context.test_kind.NORMAL);
        try {
            XTest xTest = S1.create1(c);
            Component cobj = (Component) xTest;
            l.assure(cobj.Args[0].Value == c);

            Any a1 = new Any("bla");
            Any a2 = new Any(3.14f);
            Any a3 = new Any(3.145d);
            xTest = S1.create2(c, a1, a2, a3);
            cobj = (Component) xTest;
            l.assure(cobj.Args[0].Value == c
                     && a1.Equals(cobj.Args[1])
                     && a2.Equals(cobj.Args[2])
                     && a3.Equals(cobj.Args[3]));

            bool b1 = true;
            byte b2 = 1;
            short b3 = 2;
            ushort b4 = 3;
            int b5 = 4;
            uint b6 = 5;
            long b7 = 6;
            ulong b8 = 7;
            float b9 = 0.8f;
            double b10 = 0.9;
            char b11 = 'A';
            string b12 = "BCD";
            Type b13 = typeof(ulong);
            Any b14 = new Any(22);
            Enum2 b15 = Enum2.VALUE4;
            Struct1 b16 = new Struct1(1);
            PolyStruct b17 = new PolyStruct('A', 1);
            PolyStruct b18 = new PolyStruct(new Any(true), 1);
            object b19 = new uno.util.WeakComponentBase();
            ucss.lang.XComponent b20 = (ucss.lang.XComponent) b19;
            bool b21 = b1;
            byte b22 = b2;
            short b23 = b3;
            ushort b24 = b4;
            int b25 = b5;
            uint b26 = b6;
            long b27 = b7;
            ulong b28 = b8;
            float b29 = b9;
            double b30 = b10;
            char b31 = b11;
            string b32 = b12;
            Type b33 = b13;
            Any b34 = b14;
            Enum2 b35 = b15;
            Struct1 b36 = b16;
            object b37 = b19;
            ucss.lang.XComponent b38 = b20;
            bool[] b39 = new bool[] { false, true };
            byte[] b40 = new byte[] { (byte) 1, (byte) 2 };
            short[] b41 = new short[] { (short) 123, (short) 456};
            ushort[] b42 = new ushort[] { (ushort) 789, (ushort) 101};
            int[] b43 = new int[] {1, 2, 3};
            uint[] b44 = new uint[] {4, 5, 6};
            long[] b45 = new long[] {7,8,9};
            ulong[] b46 = new ulong[] {123, 4356};
            float[] b47 = new float[] {2435f,87f};
            double[] b48 = new double[] {234d,45.2134d};
            char[] b49 = new char[] {'\u1234', 'A'};
            string[] b50 = new string[] {"a","bc"};
            Type[] b51 = new Type[] {typeof(int), typeof(long)};
            Any[] b52 = new Any[] {new Any(1), new Any("adf")};
            Enum2[] b53 = new Enum2[] {Enum2.VALUE2};
            Struct1[] b54 = new Struct1[] {new Struct1(11), new Struct1(22)};
            object[] b55 = new object[0];
            ucss.lang.XComponent[] b56 = new ucss.lang.XComponent[]{
                new uno.util.WeakComponentBase(), new uno.util.WeakComponentBase()};
            bool[][] b57 = new bool[][] {new bool[]{true,false}, new  bool[] {true}};
            byte[][] b58 = new byte[][]{new byte[] {(byte) 1}, new byte[]{(byte) 2}};
            short[][] b59 = new short[][] {new short[]{(short)6, (short)7}, new short[] {(short)9}};
            ushort[][] b60 = new ushort[][] { new ushort[]{(ushort) 11}};
            int[][] b61 = new int[][] {new int[]{1}, new int[]{2,3}, new int[]{4,5,6}};
            uint[][] b62 = new uint[][] {new uint[]{10U}, new uint[]{20U,30U}, new uint[]{40U,50U,60}};
            long[][] b63 = new long[][] {new long[]{10L}, new long[]{20L,30}, new long[]{40,50,60}};
            ulong[][] b64 = new ulong[][] { new ulong[]{10L}, new ulong[]{20L, 30L}, new ulong[]{40,50,60}};
            float[][] b65 = new float[][] {new float[]{10f}, new float[]{20f,30f}, new float[]{40f,50f,60f}};
            double[][] b66 = new double[][]{new double[]{10d}, new double[]{20d,30d}};
            char[][] b67 = new char[][] {new char[]{'a'}, new char[]{'b', 'c'}};
            string[][] b68 = new String[][] {new string[]{"a"}, new string[]{"ad", "lkj"}};
            Type[][] b69 = new Type[][] {new Type[]{typeof(byte), typeof(long)}, new Type[]{typeof(Any)}};
            Any[][] b70 = new Any[][] {new Any[]{new Any(1f), new Any(2d)}, new Any[]{new Any(34U)}};
            Enum2[][] b71 = new Enum2[][] {new Enum2[]{Enum2.VALUE2}};
            Struct1[][] b72 = new Struct1[][] {new Struct1[]{new Struct1(2), new Struct1(3)}};
            object[][] b73 =  new Object[0][];
            ucss.lang.XComponent[][] b74 = new uno.util.WeakComponentBase[0][];
            bool[][] b75 = b57;
            byte[][] b76 = b58;
            short[][] b77 = b59;
            ushort[][] b78 = b60;
            int[][] b79 = b61;
            uint[][] b80 = b62;
            long[][] b81 = b63;
            ulong[][] b82 = b64;
            float[][] b83 = b65;
            double[][] b84 = b66;
            char[][] b85 = b67;
            String[][] b86 = b68;
            Type[][] b87 =b69;
            Any[][] b88 = b70;
            Enum2[][] b89 = b71;
            Struct1[][] b90 = b72;
            Object[][] b91 = b73;
            ucss.lang.XComponent[][] b92 = b74;

            xTest = S1.create5(
                c,
                b1, b2, b3, b4, b5, b6, b7 ,b8, b9, b10,
                b11, b12, b13,
                b14,
                b15, b16, b17, b18, b19, b20,
                b21, b22, b23, b24, b25, b26, b27, b28, b29, b30,
                b31, b32, b33,
                b34,
                b35, b36, b37, b38, b39, b40,
                b41, b42, b43, b44, b45, b46, b47, b48, b49, b50,
                b51, b52, b53, b54, b55, b56, b57, b58, b59, b60,
                b61, b62, b63, b64, b65, b66, b67, b68, b69, b70,
                b71, b72, b73, b74, b75, b76, b77, b78, b79, b80,
                b81, b82, b83, b84, b85, b86, b87, b88, b89, b90,
                b91, b92
                 );

            cobj = (Component) xTest;
            l.assure(cobj.Args[0].Value == c);
            l.assure(b1.Equals(cobj.Args[1].Value));
            l.assure(b2.Equals(cobj.Args[2].Value));
            l.assure(b3.Equals(cobj.Args[3].Value));
            l.assure(b4.Equals(cobj.Args[4].Value));
            l.assure(b5.Equals(cobj.Args[5].Value));
            l.assure(b6.Equals(cobj.Args[6].Value));
            l.assure(b7.Equals(cobj.Args[7].Value));
            l.assure(b8.Equals(cobj.Args[8].Value));
            l.assure(b9.Equals(cobj.Args[9].Value));
            l.assure(b10.Equals(cobj.Args[10].Value));
            l.assure(b11.Equals(cobj.Args[11].Value));
            l.assure(b12.Equals(cobj.Args[12].Value));
            l.assure(b13.Equals(cobj.Args[13].Value));
			//Anys are not wrapped by the generated code
            l.assure(b14.Equals(cobj.Args[14]));
            l.assure(b15.Equals(cobj.Args[15].Value));
            l.assure(b16.Equals(cobj.Args[16].Value));
            l.assure(b17.Equals(cobj.Args[17].Value));
            l.assure(b18.Equals(cobj.Args[18].Value));
            l.assure(b19.Equals(cobj.Args[19].Value));
            l.assure(b20.Equals(cobj.Args[20].Value));
            l.assure(b21.Equals(cobj.Args[21].Value));
            l.assure(b22.Equals(cobj.Args[22].Value));
            l.assure(b23.Equals(cobj.Args[23].Value));
            l.assure(b24.Equals(cobj.Args[24].Value));
            l.assure(b25.Equals(cobj.Args[25].Value));
            l.assure(b26.Equals(cobj.Args[26].Value));
            l.assure(b27.Equals(cobj.Args[27].Value));
            l.assure(b28.Equals(cobj.Args[28].Value));
            l.assure(b29.Equals(cobj.Args[29].Value));
            l.assure(b30.Equals(cobj.Args[30].Value));
            l.assure(b31.Equals(cobj.Args[31].Value));
            l.assure(b32.Equals(cobj.Args[32].Value));
            l.assure(b33.Equals(cobj.Args[33].Value));
			//Anys are not wrapped by the generated code
            l.assure(b34.Equals(cobj.Args[34]));
            l.assure(b35.Equals(cobj.Args[35].Value));
            l.assure(b36.Equals(cobj.Args[36].Value));
            l.assure(b37.Equals(cobj.Args[37].Value));
            l.assure(b38.Equals(cobj.Args[38].Value));
            l.assure(b39.Equals(cobj.Args[39].Value));
            l.assure(b40.Equals(cobj.Args[40].Value));
            l.assure(b41.Equals(cobj.Args[41].Value));
            l.assure(b42.Equals(cobj.Args[42].Value));
            l.assure(b43.Equals(cobj.Args[43].Value));
            l.assure(b44.Equals(cobj.Args[44].Value));
            l.assure(b45.Equals(cobj.Args[45].Value));
            l.assure(b46.Equals(cobj.Args[46].Value));
            l.assure(b47.Equals(cobj.Args[47].Value));
            l.assure(b48.Equals(cobj.Args[48].Value));
            l.assure(b49.Equals(cobj.Args[49].Value));
            l.assure(b50.Equals(cobj.Args[50].Value));
            l.assure(b51.Equals(cobj.Args[51].Value));
            l.assure(b52.Equals(cobj.Args[52].Value));
            l.assure(b53.Equals(cobj.Args[53].Value));
            l.assure(b54.Equals(cobj.Args[54].Value));
            l.assure(b55.Equals(cobj.Args[55].Value));
            l.assure(b56.Equals(cobj.Args[56].Value));
            l.assure(b57.Equals(cobj.Args[57].Value));
            l.assure(b58.Equals(cobj.Args[58].Value));
            l.assure(b59.Equals(cobj.Args[59].Value));
            l.assure(b60.Equals(cobj.Args[60].Value));
            l.assure(b61.Equals(cobj.Args[61].Value));
            l.assure(b62.Equals(cobj.Args[62].Value));
            l.assure(b63.Equals(cobj.Args[63].Value));
            l.assure(b64.Equals(cobj.Args[64].Value));
            l.assure(b65.Equals(cobj.Args[65].Value));
            l.assure(b66.Equals(cobj.Args[66].Value));
            l.assure(b67.Equals(cobj.Args[67].Value));
            l.assure(b68.Equals(cobj.Args[68].Value));
            l.assure(b69.Equals(cobj.Args[69].Value));
            l.assure(b70.Equals(cobj.Args[70].Value));
            l.assure(b71.Equals(cobj.Args[71].Value));
            l.assure(b72.Equals(cobj.Args[72].Value));
            l.assure(b73.Equals(cobj.Args[73].Value));
            l.assure(b74.Equals(cobj.Args[74].Value));
            l.assure(b75.Equals(cobj.Args[75].Value));
            l.assure(b76.Equals(cobj.Args[76].Value));
            l.assure(b77.Equals(cobj.Args[77].Value));
            l.assure(b78.Equals(cobj.Args[78].Value));
            l.assure(b79.Equals(cobj.Args[79].Value));
            l.assure(b80.Equals(cobj.Args[80].Value));
            l.assure(b81.Equals(cobj.Args[81].Value));
            l.assure(b82.Equals(cobj.Args[82].Value));
            l.assure(b83.Equals(cobj.Args[83].Value));
            l.assure(b84.Equals(cobj.Args[84].Value));
            l.assure(b85.Equals(cobj.Args[85].Value));
            l.assure(b86.Equals(cobj.Args[86].Value));
            l.assure(b87.Equals(cobj.Args[87].Value));
            l.assure(b88.Equals(cobj.Args[88].Value));
            l.assure(b89.Equals(cobj.Args[89].Value));
            l.assure(b90.Equals(cobj.Args[90].Value));
            l.assure(b91.Equals(cobj.Args[91].Value));
            l.assure(b92.Equals(cobj.Args[92].Value));

        } catch (Exception) {
            l.assure(false);
        }

        //test
        c = new Context(Context.test_kind.NORMAL);
        try {
            
            PolyStruct2 arg1 = new PolyStruct2(typeof(PolyStruct2), 1);
            PolyStruct2 arg2 = new PolyStruct2(new Any(true), 1);
            PolyStruct2 arg3 = new PolyStruct2(true, 1);
            PolyStruct2 arg4 = new PolyStruct2((Byte)8, 1);
            PolyStruct2 arg5 = new PolyStruct2('c', 1);
            PolyStruct2 arg6 = new PolyStruct2((Int16)10, 1);
            PolyStruct2 arg7 = new PolyStruct2(11, 1);
            PolyStruct2 arg8 = new PolyStruct2(12L, 1);
            PolyStruct2 arg9 = new PolyStruct2("Hello", 1);
            PolyStruct2 arg10 = new PolyStruct2(1.3, 1);
            PolyStruct2 arg11 = new PolyStruct2(1.3d, 1);
            PolyStruct2 arg12 = new PolyStruct2(new Object(), 1);
            PolyStruct2 arg13 = new PolyStruct2(new uno.util.WeakComponentBase(), 1);
            PolyStruct2 arg14 = new PolyStruct2(
                new PolyStruct('A', 1), 1);
            PolyStruct2 arg15 = new PolyStruct2(
                new PolyStruct(new PolyStruct('A',1),1),1);
            PolyStruct arg16 = new PolyStruct("Hallo", 1);
            PolyStruct arg17 = new PolyStruct(
                new PolyStruct('A',1),1);

            Type[] arType = {typeof(PolyStruct), typeof(PolyStruct2)};
            PolyStruct2 arg101 = new PolyStruct2(arType,1);
            PolyStruct2 arg102 = new PolyStruct2(
                new Any[] {new Any(true)},1);
            PolyStruct2 arg103 = new PolyStruct2(new bool[]{true}, 1);
            PolyStruct2 arg104 = new PolyStruct2(new byte[] { (byte) 1}, 1);
            PolyStruct2 arg105 = new PolyStruct2(new char[] {'\u1234', 'A'}, 1);
            PolyStruct2 arg106 = new PolyStruct2(new short[] {(short)1}, 1);
            PolyStruct2 arg107 = new PolyStruct2(new int[] {1}, 1);
            PolyStruct2 arg108 = new PolyStruct2(new long[] {1}, 1);
            PolyStruct2 arg109 = new PolyStruct2(new string[]{"Hallo"}, 1);
            PolyStruct2 arg110 = new PolyStruct2(new float[]{1.3f}, 1);
            PolyStruct2 arg111 = new PolyStruct2(new double[] {1.3d}, 1);
            PolyStruct2 arg112 = new PolyStruct2(
                new Object[] { new Object()}, 1);
            PolyStruct2 arg113 = new PolyStruct2(
                new uno.util.WeakComponentBase[] {
                    new uno.util.WeakComponentBase()}, 1);
            PolyStruct2 arg114 = new PolyStruct2(
                new PolyStruct[]{
                new PolyStruct('A',1)} ,1);
            PolyStruct2 arg115 = new PolyStruct2(
                new PolyStruct[] {
                new PolyStruct( new PolyStruct2('A',1),1)}
                ,1);
            PolyStruct2 arg201 = new PolyStruct2(new char[][] { new char[]{'A'},
                                                            new char[]{'B'}}, 1);

			PolyStruct2[] arg301 = new PolyStruct2[] {new PolyStruct2('A', 1)};
			PolyStruct2[] arg302 = new PolyStruct2[] {new PolyStruct2(
				new PolyStruct('A', 1), 1)};
			PolyStruct2[] arg303 = new PolyStruct2[] {new PolyStruct2(
				new PolyStruct(new PolyStruct('A',1),1),1)};
			PolyStruct[] arg304 = new PolyStruct[] {new PolyStruct("Hallo", 1)};
			PolyStruct[] arg305 = new PolyStruct[] {new PolyStruct(
				new PolyStruct('A',1),1)};

			PolyStruct2[][] arg401 = new PolyStruct2[][] {new PolyStruct2[] {new PolyStruct2('A', 1)}};
			PolyStruct2[][] arg402 = new PolyStruct2[][] {new PolyStruct2[] {new PolyStruct2(
				new PolyStruct('A', 1), 1)}};
			PolyStruct2[][] arg403 = new PolyStruct2[][] {new PolyStruct2[] {new PolyStruct2(
				new PolyStruct(new PolyStruct('A',1),1),1)}};
			PolyStruct[][] arg404 = new PolyStruct[][] {new PolyStruct[] {new PolyStruct("Hallo", 1)}};
			PolyStruct[][] arg405 = new PolyStruct[][] {new PolyStruct[] {new PolyStruct(
				new PolyStruct('A',1),1)}};


            XTest xTest = S1.create6(c,
                 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,
                 arg11,arg12,arg13,arg14,arg15,arg16,arg17,
                 arg101,arg102,arg103,arg104,arg105,arg106,arg107,arg108,arg109,arg110,
                 arg111,arg112,arg113,arg114,arg115,
                 arg201,
				arg301, arg302, arg303, arg304, arg305,
				arg401, arg402, arg403, arg404, arg405);
            Component cobj = (Component) xTest;
            l.assure(cobj.Args[0].Value == c);
			//arg1 - arg17
            string sType = ((PolymorphicType) cobj.Args[1].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Type>");
            sType = ((PolymorphicType) cobj.Args[2].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<uno.Any>");
            sType = ((PolymorphicType) cobj.Args[3].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Boolean>");
            sType = ((PolymorphicType) cobj.Args[4].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Byte>");
            sType = ((PolymorphicType) cobj.Args[5].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Char>");
            sType = ((PolymorphicType) cobj.Args[6].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Int16>");
            sType = ((PolymorphicType) cobj.Args[7].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Int32>");
            sType = ((PolymorphicType) cobj.Args[8].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Int64>");
            sType = ((PolymorphicType) cobj.Args[9].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.String>");
            sType = ((PolymorphicType) cobj.Args[10].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Single>");
            sType = ((PolymorphicType) cobj.Args[11].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Double>");
            sType = ((PolymorphicType) cobj.Args[12].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Object>");
            sType = ((PolymorphicType) cobj.Args[13].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<unoidl.com.sun.star.lang.XComponent>");
            sType = ((PolymorphicType) cobj.Args[14].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any>>");
            sType = ((PolymorphicType) cobj.Args[15].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
                      "unoidl.test.cliure.climaker.PolyStruct<" +
                      "unoidl.test.cliure.climaker.PolyStruct<" +
                      "System.Char,uno.Any>,System.String>>");
            sType = ((PolymorphicType) cobj.Args[16].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct<" +
                      "System.String,unoidl.test.cliure.climaker.PolyStruct<" +
                      "System.Char,unoidl.test.cliure.climaker.PolyStruct2<" +
                      "uno.Any>>>");
            sType = ((PolymorphicType) cobj.Args[17].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct<" +
                      "unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any>," +
                      "unoidl.test.cliure.climaker.PolyStruct2<System.Char>>");
			//arg101 - arg115
            sType = ((PolymorphicType) cobj.Args[18].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Type[]>");
            sType = ((PolymorphicType) cobj.Args[19].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<uno.Any[]>");
            sType = ((PolymorphicType) cobj.Args[20].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Boolean[]>");
            sType = ((PolymorphicType) cobj.Args[21].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Byte[]>");
            sType = ((PolymorphicType) cobj.Args[22].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Char[]>");
            sType = ((PolymorphicType) cobj.Args[23].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Int16[]>");
            sType = ((PolymorphicType) cobj.Args[24].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Int32[]>");
            sType = ((PolymorphicType) cobj.Args[25].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Int64[]>");
            sType = ((PolymorphicType) cobj.Args[26].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.String[]>");
            sType = ((PolymorphicType) cobj.Args[27].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Single[]>");
            sType = ((PolymorphicType) cobj.Args[28].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Double[]>");
            sType = ((PolymorphicType) cobj.Args[29].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Object[]>");
            sType = ((PolymorphicType) cobj.Args[30].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<unoidl.com.sun.star.lang.XComponent[]>");
            sType = ((PolymorphicType) cobj.Args[31].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
                      "unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any[]>[]>");
            sType = ((PolymorphicType) cobj.Args[32].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
                                "unoidl.test.cliure.climaker.PolyStruct<" +
                                "unoidl.test.cliure.climaker.PolyStruct2<" +
                                "System.Char>,uno.Any[]>[]>");
			//arg 201
            sType = ((PolymorphicType) cobj.Args[33].Type).PolymorphicName;
            l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
                                "System.Char[][]>");
			//arg 301 - arg305
			sType = ((PolymorphicType) cobj.Args[34].Type).PolymorphicName;
			l.assure (sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Char>[]");
			sType = ((PolymorphicType) cobj.Args[35].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any>>[]");
			sType = ((PolymorphicType) cobj.Args[36].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
				"unoidl.test.cliure.climaker.PolyStruct<" +
				"unoidl.test.cliure.climaker.PolyStruct<" +
				"System.Char,uno.Any>,System.String>>[]");
			sType = ((PolymorphicType) cobj.Args[37].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct<" +
				"System.String,unoidl.test.cliure.climaker.PolyStruct<" +
				"System.Char,unoidl.test.cliure.climaker.PolyStruct2<" +
				"uno.Any>>>[]");
			sType = ((PolymorphicType) cobj.Args[38].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct<" +
				"unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any>," +
				"unoidl.test.cliure.climaker.PolyStruct2<System.Char>>[]");
			//arg 401 - arg405
			sType = ((PolymorphicType) cobj.Args[39].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<System.Char>[][]");
			sType = ((PolymorphicType) cobj.Args[40].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
				"unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any>>[][]");
			sType = ((PolymorphicType) cobj.Args[41].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct2<" +
				"unoidl.test.cliure.climaker.PolyStruct<" +
				"unoidl.test.cliure.climaker.PolyStruct<" +
				"System.Char,uno.Any>,System.String>>[][]");
			sType = ((PolymorphicType) cobj.Args[42].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct<" +
				"System.String,unoidl.test.cliure.climaker.PolyStruct<" +
				"System.Char,unoidl.test.cliure.climaker.PolyStruct2<" +
				"uno.Any>>>[][]");
			sType = ((PolymorphicType) cobj.Args[43].Type).PolymorphicName;
			l.assure( sType == "unoidl.test.cliure.climaker.PolyStruct<" +
				"unoidl.test.cliure.climaker.PolyStruct<System.Char,uno.Any>," +
				"unoidl.test.cliure.climaker.PolyStruct2<System.Char>>[][]");
			
			
			
 
        } 
		catch (Exception) 
		{
            l.assure(false);
        }
    }

    void testSingletons(Logger l)
    {
        l.Function = "testSingletons";
        Context c = new Context(Context.test_kind.NORMAL);
        try {
			XTest obj = S4.get(c);
			l.assure(obj != null);
        } catch (Exception) {
            l.assure(false);
        }

        /** Case context fails to provide singleton, a DeploymentException should be thrown.
         */
        c = new Context(Context.test_kind.CREATION_FAILED);
        try {
			XTest obj = S4.get(c);
			l.assure(obj != null);
        } catch (ucss.uno.DeploymentException e) {
            Type t = typeof(unoidl.test.cliure.climaker.S4);
            l.assure( e.Message.IndexOf(t.FullName) != -1);
        } catch (System.Exception) {
            l.assure(false);
        }
    }

    void testAttributes(Logger l)
    {
        l.Function = "testAttributes";
        //oneway attribute
        Type typeXTest = typeof(unoidl.test.cliure.climaker.XTest);
        object[] arAttr = typeXTest.GetMethod("testOneway").GetCustomAttributes(false);
        if (arAttr.Length == 1)
            l.assure(typeof(uno.OnewayAttribute).Equals(arAttr[0].GetType()));
        else
            l.assure(false);

        //test exceptions
        arAttr = typeXTest.GetMethod("testExceptions").GetCustomAttributes(false);
        if (arAttr.Length == 1 && arAttr[0].GetType() == typeof(uno.ExceptionAttribute))
        {
            uno.ExceptionAttribute attr = arAttr[0] as uno.ExceptionAttribute;
            if (attr != null && attr.Raises.Length == 2)
            {
                l.assure(attr.Raises[0] == typeof(unoidl.com.sun.star.uno.Exception));
                l.assure(attr.Raises[1] == typeof(unoidl.com.sun.star.lang.ClassNotFoundException));
            }
            else
                l.assure(false);
        }
        else
            l.assure(false);
        
        //function test must not have the oneway attribute and Exception attribute
        arAttr = typeXTest.GetMethod("test").GetCustomAttributes(false);
        l.assure(arAttr.Length == 0);

        //test exceptions on service constructor methods
        Type typeS1 = typeof(unoidl.test.cliure.climaker.S1);
        arAttr = typeS1.GetMethod("create3").GetCustomAttributes(false);
        if (arAttr.Length == 1 && arAttr[0].GetType() == typeof(uno.ExceptionAttribute))
        {
            uno.ExceptionAttribute attr = arAttr[0] as uno.ExceptionAttribute;
            if (attr != null && attr.Raises.Length == 4)
            {
                l.assure(attr.Raises[0] == typeof(unoidl.com.sun.star.uno.RuntimeException));
                l.assure(attr.Raises[1] == typeof(unoidl.com.sun.star.lang.ClassNotFoundException));
                l.assure(attr.Raises[2] == typeof(unoidl.com.sun.star.lang.IllegalAccessException));
                l.assure(attr.Raises[3] == typeof(unoidl.com.sun.star.uno.DeploymentException));
            }
            else
                l.assure(false);
        }
        else
            l.assure(false);

        //create1 does not have exceptions
        arAttr = typeS1.GetMethod("create1").GetCustomAttributes(false);
        l.assure(arAttr.Length == 0);

        //test exceptions of UNO interface attributes
        arAttr = typeXTest.GetProperty("A3").GetGetMethod().GetCustomAttributes(false);
        if (arAttr.Length == 1)
        {
            uno.ExceptionAttribute attr = arAttr[0] as uno.ExceptionAttribute;
            if (attr != null && attr.Raises.Length == 2)
            {
                l.assure(attr.Raises[0] == typeof(unoidl.com.sun.star.uno.Exception));
                l.assure(attr.Raises[1] == typeof(unoidl.com.sun.star.lang.ClassNotFoundException));
            }
            else
                l.assure(false);
        }
        else
            l.assure(false);

        arAttr = typeXTest.GetProperty("A3").GetSetMethod().GetCustomAttributes(false);
        if (arAttr.Length == 1)
        {
            uno.ExceptionAttribute attr = arAttr[0] as uno.ExceptionAttribute;
            if (attr != null && attr.Raises.Length == 1)
                l.assure(attr.Raises[0] == typeof(unoidl.com.sun.star.uno.RuntimeException));
            else
                l.assure(false);
        }
        else
            l.assure(false);

        //attribute A1 must have the ExceptionAttribute
        l.assure(typeXTest.GetProperty("A1").GetGetMethod().GetCustomAttributes(false).Length == 0);
        l.assure(typeXTest.GetProperty("A1").GetSetMethod().GetCustomAttributes(false).Length == 0);

        //Test BoundAttribute
        BoundAttribute bound = (BoundAttribute) Attribute.GetCustomAttribute(
            typeXTest.GetProperty("A1"), typeof(BoundAttribute));
        l.assure(bound != null);

        bound = (BoundAttribute) Attribute.GetCustomAttribute(
            typeXTest.GetProperty("A3"), typeof(BoundAttribute));
        l.assure(bound == null);
    }

    void testPolyStructAttributes(Logger l)
    {
        l.Function = "testPolyStructAttributes";
        //Test polymorphic struct
        Type typeStruct = typeof(unoidl.test.cliure.climaker.PolyStruct);
        object[] arAttr = typeStruct.GetCustomAttributes(false);
        if (arAttr.Length == 1)
        {
            try {
            uno.TypeParametersAttribute attr = (uno.TypeParametersAttribute) arAttr[0];
            string[] arNames = new string[]{"if", "else"};
            l.assure(attr != null && attr.Parameters.ToString().Equals(arNames.ToString()));
            }catch(Exception ) {
                l.assure(false);
            }
        }
        else
            l.assure(false);
        l.assure(typeof(unoidl.test.cliure.climaker.Struct1).GetCustomAttributes(false).Length == 0);
        //member of a polymorphic struct with a parameterized type have also an attribute
        arAttr = typeStruct.GetField("member1").GetCustomAttributes(false);
        if (arAttr.Length == 1)
        {
            uno.ParameterizedTypeAttribute attr = arAttr[0] as uno.ParameterizedTypeAttribute;
            l.assure(attr != null && attr.Type == "if");
        }
        else
            l.assure(false);

        
        //test instantiated polymorphic struct: return value        
//         Type typeXTest = typeof(XTest);
//         arAttr = typeXTest.GetMethod("testPolyStruct").ReturnTypeCustomAttributes.GetCustomAttributes(false);
//         if (arAttr.Length == 1)
//         {
//             uno.TypeArgumentsAttribute attr = arAttr[0] as uno.TypeArgumentsAttribute;
//             l.assure(attr != null && attr.Arguments.Length == 2
//                      &&attr.Arguments[0] == typeof(char)
//                      && attr.Arguments[1] == typeof(int));
//         }
//         else
//             l.assure(false);
//         arAttr = typeXTest.GetMethod("testPolyStruct").GetCustomAttributes(false);
    }

//     private XComponentContext context;

        void testPolymorphicType(Logger l)
        {
            l.Function = "testPolymorphicType";
            string name = "unoidl.test.cliure.climaker.PolyStruct<System.Int32,System.Int32>";

            uno.PolymorphicType t1 = PolymorphicType.GetType(
                typeof(unoidl.test.cliure.climaker.PolyStruct), name);

            uno.PolymorphicType t2 = PolymorphicType.GetType(
                typeof(unoidl.test.cliure.climaker.PolyStruct ), name);

            l.assure(t1 == t2);
            l.assure(t1.PolymorphicName == name);
            l.assure(t1.OriginalType == typeof(unoidl.test.cliure.climaker.PolyStruct));
            
        }

    void testInterface(Logger l)
    {
        l.Function = "testInterface";
        try {
            Context c = new Context(Context.test_kind.NORMAL);
            XTest obj = S1.create1(c);
            bool aBool = true;
            byte aByte = 0xff;
            short aShort =   0x7fff;
            ushort aUShort =   0xffff;
            int aInt  = 0x7fffffff;
            uint aUInt = 0xffffffff;
            long aLong = 0x7fffffffffffffff;
            ulong aULong = 0xffffffffffffffff;
            float aFloat = 0.314f;
            double aDouble = 0.314d;
            char aChar  = 'A';
            string aString = "Hello World";
            Type aType = typeof(XTest);
            Any aAny = new Any(typeof(XTest), obj);
            Enum2 aEnum2 = Enum2.VALUE2;
            Struct1 aStruct1 = new Struct1();
            object aXInterface = new object();
            ucss.lang.XComponent aXComponent = (ucss.lang.XComponent) obj;
            bool[] aSeqBool = {true, false, true};
            
            obj.inParameters(aBool, aByte, aShort, aUShort,
                             aInt, aUInt, aLong, aULong,
                             aFloat, aDouble, aChar, aString,
                             aType, aAny,aEnum2, aStruct1,
                             aXInterface, aXComponent, aSeqBool);

            bool outBool;
            byte outByte;
            short outShort;
            ushort outUShort;
            int outInt;
            uint outUInt;
            long outLong;
            ulong outULong;
            float outFloat;
            double outDouble;
            char outChar;
            string outString;
            Type outType;
            Any outAny;
            Enum2 outEnum2;
            Struct1 outStruct1;
            object outXInterface;
            ucss.lang.XComponent outXComponent;
            bool[] outSeqBool;

            obj.outParameters(out outBool, out outByte, out  outShort, out outUShort,
                              out outInt, out outUInt, out outLong, out outULong,
                              out outFloat, out outDouble, out outChar, out outString,
                              out outType, out outAny, out outEnum2, out outStruct1,
                              out outXInterface, out outXComponent, out outSeqBool);

            l.assure(aBool == outBool);
            l.assure(aByte == outByte);
            l.assure(aShort == outShort);
            l.assure(aUShort == outUShort);
            l.assure(aInt == outInt);
            l.assure(aUInt == outUInt);
            l.assure(aLong == outLong);
            l.assure(aULong == outULong);
            l.assure(aFloat == outFloat);
            l.assure(aDouble == outDouble);
            l.assure(aChar == outChar);
            l.assure(aString == outString);
            l.assure(aType == outType);
            l.assure(aAny.Equals(outAny));
            l.assure(aEnum2 == outEnum2);
            l.assure(aStruct1 == outStruct1);
            l.assure(aXInterface == outXInterface);
            l.assure(aXComponent == outXComponent);
            l.assure(aSeqBool == outSeqBool);

            bool inoutBool = false;
            byte inoutByte = 10;
            short inoutShort = 11;
            ushort inoutUShort = 12;
            int inoutInt = 13;
            uint inoutUInt = 14;
            long inoutLong = 15;
            ulong inoutULong = 16;
            float inoutFloat = 4.134f;
            double inoutDouble = 5.135;
            char inoutChar = 'B';
            string inoutString =  "Hello Hamburg";
            Type inoutType = typeof(int);
            Any inoutAny = new Any(inoutInt);
            Enum2 inoutEnum2 = Enum2.VALUE4;
            Struct1 inoutStruct1 = new Struct1();
            object inoutXInterface = new object();
            ucss.lang.XComponent inoutXComponent = (ucss.lang.XComponent) S1.create1(c);
            bool[] inoutSeqBool = {false, true, false};


            obj.inoutParameters(ref inoutBool, ref inoutByte, ref inoutShort, ref inoutUShort,
                                ref inoutInt, ref inoutUInt, ref inoutLong, ref inoutULong,
                                ref inoutFloat, ref inoutDouble, ref inoutChar, ref inoutString,
                                ref inoutType, ref inoutAny, ref inoutEnum2, ref inoutStruct1,
                                ref inoutXInterface, ref inoutXComponent, ref inoutSeqBool);
				
            l.assure(aBool == inoutBool);
            l.assure(aByte == inoutByte);
            l.assure(aShort == inoutShort);
            l.assure(aUShort == inoutUShort);
            l.assure(aInt == inoutInt);
            l.assure(aUInt == inoutUInt);
            l.assure(aLong == inoutLong);
            l.assure(aULong == inoutULong);
            l.assure(aFloat == inoutFloat);
            l.assure(aDouble == inoutDouble);
            l.assure(aChar == inoutChar);
            l.assure(aString == inoutString);
            l.assure(aType == inoutType);
            l.assure(aAny.Equals(inoutAny));
            l.assure(aEnum2 == inoutEnum2);
            l.assure(aStruct1 == inoutStruct1);
            l.assure(aXInterface == inoutXInterface);
            l.assure(aXComponent == inoutXComponent);
            l.assure(aSeqBool == inoutSeqBool);


            //now check the return values
            obj.inParameters(aBool, aByte, aShort, aUShort,
                             aInt, aUInt, aLong, aULong,
                             aFloat, aDouble, aChar, aString,
                             aType, aAny,aEnum2, aStruct1,
                             aXInterface, aXComponent, aSeqBool);

            l.assure(obj.retBoolean() == aBool);
            l.assure(obj.retByte() == aByte);
            l.assure(obj.retShort() == aShort);
            l.assure(obj.retUShort() == aUShort);
            l.assure(obj.retLong() == aInt);
            l.assure(obj.retULong() == aUInt);
            l.assure(obj.retHyper() == aLong);
            l.assure(obj.retUHyper() == aULong);
            l.assure(obj.retFloat() == aFloat);
            l.assure(obj.retDouble() == aDouble);
            l.assure(obj.retChar() == aChar);
            l.assure(obj.retString() == aString);
            l.assure(obj.retType() == aType);
            l.assure(obj.retAny().Equals(aAny));
            l.assure(obj.retEnum() == aEnum2);
            l.assure(obj.retStruct1() == aStruct1);
            l.assure(obj.retXInterface() == aXInterface);
            l.assure(obj.retXComponent() == aXComponent);
            l.assure(obj.retSeqBool() == aSeqBool);


            obj = S1.create1(c);
            obj.attrBoolean = true;
            l.assure(obj.attrBoolean == true);
            obj.attrByte = aByte;
            l.assure(obj.attrByte == aByte);
            obj.attrShort = aShort;
            l.assure(obj.attrShort == aShort);
            obj.attrUShort = aUShort;
            l.assure(obj.attrUShort == aUShort);
            obj.attrLong = aInt;
            l.assure(obj.attrLong == aInt);
            obj.attrULong = aUInt;
            l.assure(obj.attrULong == aUInt);
            obj.attrHyper = aLong;
            l.assure(obj.attrHyper == aLong);
            obj.attrUHyper = aULong;
            l.assure(obj.attrUHyper == aULong);
            obj.attrFloat = aFloat;
            l.assure(obj.attrFloat == aFloat);
            obj.attrDouble = aDouble;
            l.assure(obj.attrDouble == aDouble);
            obj.attrChar = aChar;
            l.assure(obj.attrChar == aChar);
            obj.attrString = aString;
            l.assure(obj.attrString == aString);
            obj.attrType = aType;
            l.assure(obj.attrType == aType);
            obj.attrAny = aAny;
            l.assure(obj.attrAny.Equals(aAny));
            obj.attrEnum2 = aEnum2;
            l.assure(obj.attrEnum2 == aEnum2);
            obj.attrStruct1 = aStruct1;
            l.assure(obj.attrStruct1 == aStruct1);
            obj.attrXInterface = aXInterface;
            l.assure(obj.attrXInterface == aXInterface);
            obj.attrXComponent = aXComponent;
            l.assure(obj.attrXComponent == aXComponent);
            obj.attrSeqBoolean = aSeqBool;
            l.assure(obj.attrSeqBoolean == aSeqBool);
        } catch (Exception )
        {
            l.assure(false);
        }
    }

    public void testAny(Logger l)
    {
        l.Function = "testAny";
        //create any with valid and invalid arguments
        try
        {
            Any a = new Any(null, null);
            l.assure(false);
        }
        catch(System.Exception e)
        {
            l.assure(e.Message.IndexOf("Any") != -1);
        }
        try
        {
            Any a = new Any(typeof(int), null);
            l.assure(false);
        }
        catch(System.Exception e)
        {
            l.assure(e.Message.IndexOf("Any") != -1);
        }
        

        try
        {
            Any a = new Any(typeof(unoidl.com.sun.star.uno.XComponentContext), null);
            a = new Any('a');
            a = new Any((sbyte)1);
        }
        catch (System.Exception)
        {
            l.assure(false);
        }

        //test polymorphic struct
        try
        {
            Any a = new Any(typeof(unoidl.test.cliure.climaker.PolyStruct),
                            new PolyStruct());
            l.assure(false);
        }
        catch (System.Exception e)
        {
            l.assure(e.Message.IndexOf("Any") != -1);
        }
        try
        {
            Any a = new Any(uno.PolymorphicType.GetType(
                                typeof(unoidl.test.cliure.climaker.PolyStruct),
                                "unoidl.test.cliure.climaker.PolyStruct<System.Char>"),
                            new PolyStruct('A', 10));
        }
        catch (System.Exception )
        {
            l.assure(false);
        }

        //test Any.Equals

        Any aVoid = Any.VOID;
        l.assure(aVoid.Equals((object) Any.VOID));
        l.assure(aVoid.Equals(Any.VOID));

        l.assure(aVoid.Equals(new Any("")) == false);

        Any a1 = new Any(10);
        Any a2 = a1;
        l.assure(a1.Equals(a2));

        a1 = new Any(typeof(unoidl.com.sun.star.uno.XComponentContext), null);
        l.assure(a1.Equals(a2) == false);
        a2 = a1;
        l.assure(a1.Equals(a2));
        l.assure(a1.Equals(null) == false);
        l.assure(a1.Equals(new object()) == false);
    }
}
