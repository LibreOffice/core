package com.sun.star.uno;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XTypeProvider;
import java.util.Vector;

public class AnyConverter_Test
{
    Any anyBool; //
    Any anyChar; //
    Any anyByte; //
    Any anyShort; //
    Any anyInt; //
    Any anyLong; //
    Any anyFloat; //
    Any anyDouble; //
    Any anyObj; //
    Any anyStr; //
    Any anyType; //
    Any anyArByte; //
    Any anyVoid;   //
    Any anyXTypeProvider;

    Boolean aBool= new Boolean(true);
    Character aChar= new Character('A');
    Byte aByte= new Byte((byte) 111);
    Short aShort= new Short((short) 11111);
    Integer aInt= new Integer( 1111111);
    Long aLong= new Long( 0xffffffff);
    Float aFloat= new Float( 3.14);
    Double aDouble= new Double( 3.145);
    Object aObj= new ATypeProvider();
    String aStr= new String("I am a string");
    Type aType= new Type(String.class);
    byte[] arByte= new byte[] {1,2,3};

    public AnyConverter_Test() {
        anyVoid= new Any(new Type(void.class), null);
        anyBool= new Any(new Type(Boolean.TYPE), aBool);
        anyChar= new Any(new Type(Character.TYPE), aChar);
        anyByte= new Any(new Type(Byte.TYPE), aByte);
        anyShort= new Any(new Type(Short.TYPE), aShort);
        anyInt= new Any(new Type(Integer.TYPE), aInt);
        anyLong= new Any(new Type(Long.TYPE), aLong);
        anyFloat= new Any(new Type(Float.TYPE), aFloat);
        anyDouble= new Any(new Type(Double.TYPE), aDouble);
        anyObj= new Any(new Type(Object.class) /* any */, aObj);
        anyStr= new Any(new Type(String.class), aStr);
        anyType= new Any(new Type(Type.class), aType);
        anyArByte= new Any(new Type(byte[].class), arByte);
        anyXTypeProvider= new Any(new Type(XTypeProvider.class), aObj);
    }
    /*Allowed arguments: Boolean object or an Any object containing a Boolean object.*/
    public boolean test_toBoolean() {
        System.out.println("Testing AnyConverter.toBoolean");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            boolean b= AnyConverter.toBoolean(aBool);
            r[i++]= b == aBool.booleanValue() ? true : false;
            b= AnyConverter.toBoolean(anyBool);
            r[i++]= b == ((Boolean)anyBool.getObject()).booleanValue() ? true : false;
            // must fail
            try { AnyConverter.toBoolean(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toBoolean(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}

        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /* Allowed arguments: Character and Any containing a Character */
    public boolean test_toChar() {
        System.out.println("Testing AnyConverter.toChar");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            char b= AnyConverter.toChar(aChar);
            r[i++]= b == aChar.charValue() ? true : false;
            b= AnyConverter.toChar(anyChar);
            r[i++]= b == ((Character)anyChar.getObject()).charValue() ? true : false;
            // must fail
            try { AnyConverter.toChar(aBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    /*Allowed arguments: Byte and Any containing a Byte */
    public boolean test_toByte() {
        System.out.println("Testing AnyConverter.toByte");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            byte val= AnyConverter.toByte(aByte);
            r[i++]= val == aByte.byteValue() ? true : false;
            val= AnyConverter.toByte(anyByte);
            r[i++]= val == ((Byte)anyByte.getObject()).byteValue() ? true : false;
            // must fail
            try { AnyConverter.toByte(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toByte(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}

        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed argument types are Byte, Short or Any containing these types.*/
    public boolean test_toShort() {
        System.out.println("Testing AnyConverter.toShort");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            //must work
            short sh= AnyConverter.toShort(aByte);
            r[i++]= sh == aByte.byteValue() ? true : false;
            sh= AnyConverter.toShort(aShort);
            r[i++]= sh == aShort.shortValue() ? true : false;
            sh= AnyConverter.toShort(anyByte);
            r[i++]= sh == ((Byte)anyByte.getObject()).byteValue() ? true : false;
            sh= AnyConverter.toShort(anyShort);
            r[i++]= sh == ((Short) anyShort.getObject()).shortValue() ? true : false;

            Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
            r[i++]= (5 == AnyConverter.toUnsignedShort( a ));
            try { AnyConverter.toShort(a); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toUnsignedShort(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}

           // must fail with a com.sun.star.lang.IllegalArgumentException
            try { AnyConverter.toChar(aBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toChar(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toShort(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed argument: Byte, Short, Integer or Any containing these types*/
    public boolean test_toInt() {
        System.out.println("Testing AnyConverter.toInt");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            //must work
            int val= AnyConverter.toInt(aByte);
            r[i++]= val == aByte.byteValue() ? true : false;
            val= AnyConverter.toInt(aShort);
            r[i++]= val == aShort.shortValue() ? true : false;
            val= AnyConverter.toInt(aInt);
            r[i++]= val == aInt.intValue() ? true : false;
            val= AnyConverter.toInt(anyByte);
            r[i++]= val == ((Byte)anyByte.getObject()).byteValue() ? true : false;
            val= AnyConverter.toInt(anyShort);
            r[i++]= val == ((Short) anyShort.getObject()).shortValue() ? true : false;
            val= AnyConverter.toInt(anyInt);
            r[i++]= val == ((Integer) anyInt.getObject()).intValue() ? true : false;

            Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
            r[i++]= (5 == AnyConverter.toInt(a));
            r[i++]= (5 == AnyConverter.toUnsignedInt(a));
            try { AnyConverter.toUnsignedInt(anyInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
            r[i++]= (5 == AnyConverter.toUnsignedInt(a));
            try { AnyConverter.toInt(a); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toUnsignedInt(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}

           // must fail with a com.sun.star.lang.IllegalArgumentException
            try { AnyConverter.toInt(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toInt(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed argument: Byte, Short, Integer, Long or Any containing these types*/
    public boolean test_toLong() {
        System.out.println("Testing AnyConverter.toLong");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            //must work
            long val= AnyConverter.toLong(aByte);
            r[i++]= val == aByte.byteValue() ? true : false;
            val= AnyConverter.toLong(aShort);
            r[i++]= val == aShort.shortValue() ? true : false;
            val= AnyConverter.toLong(aInt);
            r[i++]= val == aInt.intValue() ? true : false;
            val= AnyConverter.toLong(aLong);
            r[i++]= val == aLong.longValue() ? true : false;
            val= AnyConverter.toLong(anyByte);
            r[i++]= val == ((Byte)anyByte.getObject()).byteValue() ? true : false;
            val= AnyConverter.toLong(anyShort);
            r[i++]= val == ((Short) anyShort.getObject()).shortValue() ? true : false;
            val= AnyConverter.toLong(anyInt);
            r[i++]= val == ((Integer) anyInt.getObject()).intValue() ? true : false;
            val= AnyConverter.toLong(anyLong);
            r[i++]= val == ((Long) anyLong.getObject()).longValue() ? true : false;

            Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
            r[i++]= (5 == AnyConverter.toLong(a));
            r[i++]= (5 == AnyConverter.toUnsignedLong(a));
            try { AnyConverter.toUnsignedLong(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
            r[i++]= (5 == AnyConverter.toUnsignedLong(a));
            r[i++]= (5 == AnyConverter.toLong(a));
            try { AnyConverter.toUnsignedLong(anyInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            a = new Any( Type.UNSIGNED_HYPER, new Long(5) );
            r[i++]= (5 == AnyConverter.toUnsignedLong(a));
            try { AnyConverter.toLong(a); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toUnsignedLong(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}

           // must fail with a com.sun.star.lang.IllegalArgumentException
            try { AnyConverter.toLong(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toLong(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                System.err.println( "[toLong()] r[" + c + "] == false : failed!" );
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed argument: Byte, Short, Float or Any containing these types.*/
    public boolean test_toFloat() {
        System.out.println("Testing AnyConverter.toFloat");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            //must work
            float val= AnyConverter.toFloat(aByte);
            r[i++]= val == aByte.byteValue() ? true : false; // 111 = 111.0
            val= AnyConverter.toFloat(anyByte);
            r[i++]= val == ((Byte)anyByte.getObject()).byteValue() ? true : false;
            val= AnyConverter.toFloat(aShort);
            r[i++]= val == aShort.shortValue() ? true : false;//11111 = 11111.0
            val= AnyConverter.toFloat(anyShort);
            r[i++]= val == ((Short) anyShort.getObject()).shortValue() ? true : false;
            val= AnyConverter.toFloat(aFloat);
            r[i++]= val == aFloat.floatValue() ? true : false;
            val= AnyConverter.toFloat(anyFloat);
            r[i++]= val == ((Float) anyFloat.getObject()).floatValue() ? true : false;
           // must fail with a com.sun.star.lang.IllegalArgumentException
            try { AnyConverter.toFloat(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toFloat(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    /*Allowed argument: Byte, Short, Int, Float, Double or Any containing these types.*/
    public boolean test_toDouble() {
        System.out.println("Testing AnyConverter.toDouble");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            //must work
            double val= AnyConverter.toDouble(aByte);
            r[i++]= val == aByte.byteValue() ? true : false; // 111 = 111.0
            val= AnyConverter.toDouble(anyByte);
            r[i++]= val == ((Byte)anyByte.getObject()).byteValue() ? true : false;
            val= AnyConverter.toDouble(aShort);
            r[i++]= val == aShort.shortValue() ? true : false;//11111 = 11111.0
            val= AnyConverter.toDouble(anyShort);
            r[i++]= val == ((Short) anyShort.getObject()).shortValue() ? true : false;
            val= AnyConverter.toDouble(aInt);
            r[i++]= val == aInt.intValue() ? true : false;
            val= AnyConverter.toDouble(anyInt);
            r[i++]= val == ((Integer) anyInt.getObject()).intValue() ? true : false;
            val= AnyConverter.toDouble(aFloat);
            r[i++]= val == aFloat.floatValue() ? true : false;
            val= AnyConverter.toDouble(anyFloat);
            float float1= ((Float) anyFloat.getObject()).floatValue();
            r[i++]= val <= (float1 + 0.1) ||
                    val >= (float1 - 0.1) ? true : false;
            val= AnyConverter.toDouble(aDouble);
            r[i++]= val == aDouble.doubleValue() ? true : false;
            val= AnyConverter.toDouble(anyDouble);
            r[i++]= val == ((Double) anyDouble.getObject()).doubleValue() ? true : false;
           // must fail with a com.sun.star.lang.IllegalArgumentException
            try { AnyConverter.toDouble(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toDouble(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed arguments: The Object or an Any object containing The Object.
     The Boolean, Integer, Long etc. implement interfaces. If they are supplied as arguments then, the method
     returns null instead of throwing an exception*/
    public boolean test_toObject() {
        System.out.println("Testing AnyConverter.toObject");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            Type _type= new Type(XTypeProvider.class);
            Object val= AnyConverter.toObject(_type, aObj);
            r[i++]= UnoRuntime.areSame(val, aObj);
            val= AnyConverter.toObject(_type, anyObj);
            r[i++]= UnoRuntime.areSame(val, anyObj.getObject());
            val= AnyConverter.toObject(_type, new Any( new Type(XTypeProvider.class), null));
            r[i++]= val == null;

            // structs, exceptions
            com.sun.star.lang.IllegalArgumentException exc =
                new com.sun.star.lang.IllegalArgumentException();
            Any any_exc = new Any(
                new Type( "com.sun.star.lang.IllegalArgumentException", TypeClass.EXCEPTION ), exc );
            r[i++] = AnyConverter.toObject( new Type( com.sun.star.lang.IllegalArgumentException.class ), any_exc ).equals( exc );
            r[i++] = AnyConverter.toObject( new Type( com.sun.star.uno.Exception.class ), any_exc ).equals( exc );
            try { AnyConverter.toObject( new Type( com.sun.star.uno.RuntimeException.class ), any_exc ); } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i]=true;};
            ++i;
            any_exc = new Any( com.sun.star.lang.IllegalArgumentException.class, exc );
            r[i++] = AnyConverter.toObject( new Type( com.sun.star.lang.IllegalArgumentException.class ), any_exc ).equals( exc );
            r[i++] = AnyConverter.toObject( new Type( com.sun.star.uno.Exception.class ), any_exc ).equals( exc );
            try { AnyConverter.toObject( new Type( com.sun.star.uno.RuntimeException.class ), any_exc ); } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i]=true;};
            ++i;

            // must fail
            try { AnyConverter.toObject(_type, aType); } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i]=true;}
            ++i;
            try { AnyConverter.toObject(_type, anyType); } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i]=true;}
            ++i;
            try { AnyConverter.toObject(_type, anyVoid); } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i]=true;}
            ++i;
            try { AnyConverter.toObject(_type, new Object()); } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i]=true;}
            ++i;

        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                System.err.println( "[toObject()] r[" + c + "] == false : failed!" );
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed arguments: String or an Any object containing a Boolean object.*/
    public boolean test_toString() {
        System.out.println("Testing AnyConverter.toString");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            String val= AnyConverter.toString(aStr);
            r[i++]= aStr.equals(val);
            val= AnyConverter.toString(anyStr);
            r[i++]= ((String)anyStr.getObject()).equals(val);
            // must fail
            try { AnyConverter.toString(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(aType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyType); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toString(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}

        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    /*Allowed arguments: Type object or an Any object containing a Boolean object.*/
    public boolean test_toType() {
        System.out.println("Testing AnyConverter.toType");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            Type val= AnyConverter.toType(aType);
            r[i++]= val == aType ? true : false;
            val= AnyConverter.toType(anyType);
            r[i++]= val == anyType.getObject() ? true : false;
            // must fail
            try { AnyConverter.toType(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    /*Allowed arguments: Type object or an Any object containing a Boolean object.*/
    public boolean test_toArray() {
        System.out.println("Testing AnyConverter.toArray");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            Object val= AnyConverter.toArray(arByte);
            r[i++]= val == arByte ? true : false;
            val= AnyConverter.toArray(anyArByte);
            r[i++]= val == anyArByte.getObject() ? true : false;
            // must fail
            try { AnyConverter.toType(aBool); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyBool); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyChar); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyShort); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aInt); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyInt); i++; } catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyLong); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyFloat); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyDouble); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyObj); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(aStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyStr); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyVoid); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(arByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
            try { AnyConverter.toType(anyArByte); i++;} catch (com.sun.star.lang.IllegalArgumentException ie) {r[i++]=true;}
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isBoolean() {
        System.out.println("Testing AnyConverter.isBoolean");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isBoolean(aBool);
            r[i++]= AnyConverter.isBoolean(anyBool);
            // must fail
            r[i++]= AnyConverter.isBoolean(aChar) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    public boolean test_isChar() {
        System.out.println("Testing AnyConverter.isChar");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isChar(aChar);
            r[i++]= AnyConverter.isChar(anyChar);
            // must fail
            r[i++]= AnyConverter.isChar(aBool) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    public boolean test_isByte() {
        System.out.println("Testing AnyConverter.isByte");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isByte(aByte);
            r[i++]= AnyConverter.isByte(anyByte);
            // must fail
            r[i++]= AnyConverter.isByte(aBool) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isShort() {
        System.out.println("Testing AnyConverter.isShort");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isShort(aShort);
            r[i++]= AnyConverter.isShort(anyShort);
            r[i++]= Type.SHORT.equals( AnyConverter.getType(anyShort) );

            Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
            r[i++]= Type.UNSIGNED_SHORT.equals( AnyConverter.getType(a) );

            // must fail
            r[i++]= AnyConverter.isShort(a) ? false : true;
            r[i++]= ! Type.SHORT.equals( AnyConverter.getType(a) );
            r[i++]= AnyConverter.isShort(aBool) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isInt() {
        System.out.println("Testing AnyConverter.isInt");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isInt(aInt);
            r[i++]= AnyConverter.isInt(anyInt);
            r[i++]= Type.LONG.equals( AnyConverter.getType(anyInt) );

            Any a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
            r[i++]= Type.UNSIGNED_LONG.equals( AnyConverter.getType(a) );

            // must fail
            r[i++]= AnyConverter.isInt(a) ? false : true;
            r[i++]= ! Type.LONG.equals( AnyConverter.getType(a) );
            r[i++]= AnyConverter.isInt(aBool) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isLong() {
        System.out.println("Testing AnyConverter.isLong");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isLong(aLong);
            r[i++]= AnyConverter.isLong(anyLong);
            r[i++]= Type.HYPER.equals( AnyConverter.getType(anyLong) );

            Any a = new Any( Type.UNSIGNED_HYPER, new Long(5) );
            r[i++]= Type.UNSIGNED_HYPER.equals( AnyConverter.getType(a) );

            // must fail
            r[i++]= AnyConverter.isLong(a) ? false : true;
            r[i++]= ! Type.HYPER.equals( AnyConverter.getType(a) );
            r[i++]= AnyConverter.isLong(aBool) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isFloat() {
        System.out.println("Testing AnyConverter.isFloat");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isFloat(aFloat);
            r[i++]= AnyConverter.isFloat(anyFloat);
            // must fail
            r[i++]= AnyConverter.isFloat(aDouble) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isDouble() {
        System.out.println("Testing AnyConverter.isDouble");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isDouble(aDouble);
            r[i++]= AnyConverter.isDouble(anyDouble);
            // must fail
            r[i++]= AnyConverter.isDouble(aFloat) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    public boolean test_isObject() {
        System.out.println("Testing AnyConverter.isObject");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isObject(aObj);
            r[i++]= AnyConverter.isObject(anyObj);
            r[i++]= AnyConverter.isObject( new Any( XInterface.class, null));
            // must fail
            r[i++]= AnyConverter.isObject(new Object()) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                System.err.println( "[isObject()] r[" + c + "] == false : failed!" );
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }
    public boolean test_isString() {
        System.out.println("Testing AnyConverter.isString");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isString(aStr);
            r[i++]= AnyConverter.isString(anyStr);
            // must fail
            r[i++]= AnyConverter.isString(new Object()) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isType() {
        System.out.println("Testing AnyConverter.isType");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isType(aType);
            r[i++]= AnyConverter.isType(anyType);
            // must fail
            r[i++]= AnyConverter.isType(new Object()) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isArray() {
        System.out.println("Testing AnyConverter.isArray");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isArray(arByte);
            r[i++]= AnyConverter.isArray(anyArByte);
            // must fail
            r[i++]= AnyConverter.isArray(new Object()) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    public boolean test_isVoid() {
        System.out.println("Testing AnyConverter.isVoid");
        boolean ret= false;
        boolean r[]= new boolean[50];
        int i=0;

        try {
            // must work
            r[i++]= AnyConverter.isVoid(anyVoid);
            // must fail
            r[i++]= AnyConverter.isVoid(new Object()) ? false : true;
        } catch (java.lang.Exception e) {
            i++;
        }
        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Failed");
        else
        {
            System.out.println("Passed");
            ret= true;
        }
        return ret;
    }

    static public boolean test(Vector notpassed) throws java.lang.Exception
    {
                AnyConverter_Test o= new AnyConverter_Test();
        boolean r[]= new boolean[50];
        int i= 0;
        r[i++]= o.test_toBoolean();
        r[i++]= o.test_toChar();
        r[i++]= o.test_toByte();
        r[i++]= o.test_toShort();
        r[i++]= o.test_toInt();
        r[i++]= o.test_toLong();
        r[i++]= o.test_toFloat();
        r[i++]= o.test_toDouble();
        r[i++]= o.test_toObject();
        r[i++]= o.test_toString();
        r[i++]= o.test_toType();
        r[i++]= o.test_toArray();
        r[i++]= o.test_isBoolean();
        r[i++]= o.test_isChar();
        r[i++]= o.test_isByte();
        r[i++]= o.test_isShort();
        r[i++]= o.test_isInt();
        r[i++]= o.test_isLong();
        r[i++]= o.test_isFloat();
        r[i++]= o.test_isDouble();
        r[i++]= o.test_isObject();
        r[i++]= o.test_isString();
        r[i++]= o.test_isType();
        r[i++]= o.test_isArray();
        r[i++]= o.test_isVoid();

        boolean bError= false;
        for (int c= 0; c < i; c++) {
            if (r[c] == false) {
                bError= true;
                break;
            }
        }
        if ( bError )
            System.out.println("Error occured");
        else
            System.out.println("No errors");

        if (bError == false && notpassed != null)
            notpassed.addElement("AnyConverter test failed!");
        return bError;
    }

    static public void main(String[] args) {
        try {
           AnyConverter_Test.test(null);
        }catch( java.lang.Exception e) {
        }
//        AnyConverter_Test o= new AnyConverter_Test();
//        boolean r[]= new boolean[50];
//        int i= 0;
//        r[i++]= o.test_toBoolean();
//        r[i++]= o.test_toChar();
//        r[i++]= o.test_toByte();
//        r[i++]= o.test_toShort();
//        r[i++]= o.test_toInt();
//        r[i++]= o.test_toLong();
//        r[i++]= o.test_toFloat();
//        r[i++]= o.test_toDouble();
//        r[i++]= o.test_toObject();
//        r[i++]= o.test_toString();
//        r[i++]= o.test_toType();
//        r[i++]= o.test_toArray();
//        r[i++]= o.test_isBoolean();
//        r[i++]= o.test_isChar();
//        r[i++]= o.test_isByte();
//        r[i++]= o.test_isShort();
//        r[i++]= o.test_isInt();
//        r[i++]= o.test_isLong();
//        r[i++]= o.test_isFloat();
//        r[i++]= o.test_isDouble();
//        r[i++]= o.test_isObject();
//        r[i++]= o.test_isString();
//        r[i++]= o.test_isType();
//        r[i++]= o.test_isArray();
//        r[i++]= o.test_isVoid();
//
//        boolean bError= false;
//        for (int c= 0; c < i; c++) {
//            if (r[c] == false) {
//                bError= true;
//                break;
//            }
//        }
//        if ( bError )
//            System.out.println("Error occured");
//        else
//            System.out.println("No errors");

    }
}


class ATypeProvider implements com.sun.star.lang.XTypeProvider
{

    public byte[] getImplementationId()
    {
        return new byte[]{1,2,3};
    }

    public com.sun.star.uno.Type[] getTypes()
    {
        return new Type[]{new Type(XTypeProvider.class)};
    }

}
