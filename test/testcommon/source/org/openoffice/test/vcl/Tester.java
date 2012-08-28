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



package org.openoffice.test.vcl;

import java.awt.AWTException;
import java.awt.Robot;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.util.HashMap;
import java.util.StringTokenizer;


/**
 *
 */
public class Tester {
    static Robot robot;

    static double factor = Double.parseDouble(System.getProperty("sleep.factor", "1.0"));

    static {
        try {
            robot = new Robot();
            robot.setAutoDelay(10);
            robot.setAutoWaitForIdle(true);
        } catch (AWTException e) {
            e.printStackTrace();
        }
    }

    public Tester() {

    }

    /**
     *
     * @param delay
     */
    public static void sleep(double seconds) {
        try {
            Thread.sleep((long) (seconds * factor * 1000));
        } catch (InterruptedException e) {
        }
    }

    /**
     * Click on the screen
     * @param x
     * @param y
     */
    public static void click(int x, int y) {
        robot.mouseMove(x, y);
        robot.mousePress(InputEvent.BUTTON1_MASK);
        robot.mouseRelease(InputEvent.BUTTON1_MASK);
        robot.delay(100);
    }

    public static void doubleClick(int x, int y) {
        robot.mouseMove(x, y);
        robot.mousePress(InputEvent.BUTTON1_MASK);
        robot.mouseRelease(InputEvent.BUTTON1_MASK);
        robot.mousePress(InputEvent.BUTTON1_MASK);
        robot.mouseRelease(InputEvent.BUTTON1_MASK);
        robot.delay(100);
    }

    /**
     * Right click on the screen
     * @param x
     * @param y
     */
    public static void rightClick(int x, int y) {
        robot.mouseMove(x, y);
        robot.mousePress(InputEvent.BUTTON3_MASK);
        robot.mouseRelease(InputEvent.BUTTON3_MASK);
        robot.delay(100);
    }

    public static void drag(int fromX, int fromY, int toX, int toY) {
        robot.mouseMove(fromX, fromY);
        robot.mousePress(InputEvent.BUTTON1_MASK);
        int x = fromX;
        int y = fromY;
        int dx = toX > fromX ? 1 : -1;
        int dy = toY > fromY ? 1 : -1;
        while (x != toX || y != toY) {
            if (x != toX)
                x = x + dx;
            if (y != toY)
                y = y + dy;
            robot.mouseMove(x, y);
        }
        robot.mouseRelease(InputEvent.BUTTON1_MASK);
        robot.delay(100);
    }

    /**
     * Type String
     * @param text
     */
    public static void typeText(String text) {
        for (int i = 0; i < text.length(); i++) {
            char ch = text.charAt(i);
            String[] shiftKeys = keyMap.get(ch);
            if (shiftKeys == null)
                throw new RuntimeException("Keys is invalid!");
            typeShortcut(shiftKeys);
        }
    }

    /**
     * Type shortcut
     * @param keys
     */
    public static void typeShortcut(String... keys) {
        for(int i = 0; i < keys.length; i++) {
            String key = keys[i];
            key = key.toLowerCase();
            Integer keyCode = keyCodeMap.get(key);
            if (keyCode == null)
                throw new RuntimeException("Invalid keys!");
            robot.keyPress(keyCode);
        }

        for(int i = keys.length - 1; i >= 0; i--) {
            String key = keys[i];
            key = key.toLowerCase();
            Integer keyCode = keyCodeMap.get(key);
            if (keyCode == null)
                throw new RuntimeException("Invalid keys!");
            robot.keyRelease(keyCode);
        }
    }

    /**
     * Type the keys
     * To input shortcut, use "<key key key ...>". The keys is separated with space and surrounded with angle brackets.
     * For example, input the word "hello" and then press Ctrl+A to select the all content.
     * typeKeys("hello<ctrl a>");
     *
     *
     *
     * @param keys
     */
    public static void typeKeys(String keys) {
        StringTokenizer tokenizer = new StringTokenizer(keys, "<>", true);
        int state = 0;
        while (tokenizer.hasMoreTokens()) {
            String token = tokenizer.nextToken();
            switch (state) {
            case 0:
                if ("<".equals(token)) {
                    state = 1;
                } else if (">".equals(token)) {
                    throw new RuntimeException("Invalid keys!");
                } else {
                    typeText(token);
                }
            break;
            case 1:
                if (">".equals(token)) {
                    state = 0;
                } else if ("<".equals(token)){
                    throw new RuntimeException("Invalid keys!");
                } else {
                    if (token.startsWith("$")) {
                        String[] ckeys = customizedMap.get(token.substring(1));
                        if (ckeys == null || ckeys.length == 0)
                            throw new RuntimeException(token + " is not a customized shortcut!");
                        typeShortcut(ckeys);
                    } else {
                        String[] ckeys = token.split(" ");
                        typeShortcut(ckeys);
                    }
                }
            }
        }
    }
    protected static HashMap<String, Integer> keyCodeMap = new HashMap<String, Integer>();
    protected static HashMap<Character, String[]> keyMap = new HashMap<Character, String[]>();
    protected static HashMap<String, String[]> customizedMap = new HashMap<String, String[]>();
    static {
        //US keyboard
        keyCodeMap.put("esc", KeyEvent.VK_ESCAPE);
        keyCodeMap.put("f1", KeyEvent.VK_F1);
        keyCodeMap.put("f2", KeyEvent.VK_F2);
        keyCodeMap.put("f3", KeyEvent.VK_F3);
        keyCodeMap.put("f4", KeyEvent.VK_F4);
        keyCodeMap.put("f5", KeyEvent.VK_F5);
        keyCodeMap.put("f6", KeyEvent.VK_F6);
        keyCodeMap.put("f7", KeyEvent.VK_F7);
        keyCodeMap.put("f8", KeyEvent.VK_F8);
        keyCodeMap.put("f9", KeyEvent.VK_F9);
        keyCodeMap.put("f10", KeyEvent.VK_F10);
        keyCodeMap.put("f11", KeyEvent.VK_F11);
        keyCodeMap.put("f12", KeyEvent.VK_F12);
        keyCodeMap.put("printscreen", KeyEvent.VK_PRINTSCREEN);
        keyCodeMap.put("scrolllock", KeyEvent.VK_SCROLL_LOCK);
        keyCodeMap.put("pause", KeyEvent.VK_PAUSE);
        keyCodeMap.put("tab", KeyEvent.VK_TAB);
        keyCodeMap.put("space", KeyEvent.VK_SPACE);
        keyCodeMap.put("capslock", KeyEvent.VK_CAPS_LOCK);
        keyCodeMap.put("shift", KeyEvent.VK_SHIFT);
        keyCodeMap.put("ctrl", KeyEvent.VK_CONTROL);
        keyCodeMap.put("alt", KeyEvent.VK_ALT);
        keyCodeMap.put("bs", KeyEvent.VK_BACK_SPACE);
        keyCodeMap.put("backspace", KeyEvent.VK_BACK_SPACE);
        keyCodeMap.put("enter", KeyEvent.VK_ENTER);
        keyCodeMap.put("cr", KeyEvent.VK_ENTER);
        keyCodeMap.put("command", 157);
        keyCodeMap.put("control", KeyEvent.VK_CONTROL);
        keyCodeMap.put("insert", KeyEvent.VK_INSERT);
        keyCodeMap.put("del", KeyEvent.VK_DELETE);
        keyCodeMap.put("delete", KeyEvent.VK_DELETE);
        keyCodeMap.put("home", KeyEvent.VK_HOME);
        keyCodeMap.put("end", KeyEvent.VK_END);
        keyCodeMap.put("pageup", KeyEvent.VK_PAGE_UP);
        keyCodeMap.put("pagedown", KeyEvent.VK_PAGE_DOWN);
        keyCodeMap.put("up", KeyEvent.VK_UP);
        keyCodeMap.put("left", KeyEvent.VK_LEFT);
        keyCodeMap.put("right", KeyEvent.VK_RIGHT);
        keyCodeMap.put("down", KeyEvent.VK_DOWN);
        keyCodeMap.put("numlock", KeyEvent.VK_NUM_LOCK);
        keyCodeMap.put("a", KeyEvent.VK_A);
        keyCodeMap.put("b", KeyEvent.VK_B);
        keyCodeMap.put("c", KeyEvent.VK_C);
        keyCodeMap.put("d", KeyEvent.VK_D);
        keyCodeMap.put("e", KeyEvent.VK_E);
        keyCodeMap.put("f", KeyEvent.VK_F);
        keyCodeMap.put("g", KeyEvent.VK_G);
        keyCodeMap.put("h", KeyEvent.VK_H);
        keyCodeMap.put("i", KeyEvent.VK_I);
        keyCodeMap.put("j", KeyEvent.VK_J);
        keyCodeMap.put("k", KeyEvent.VK_K);
        keyCodeMap.put("l", KeyEvent.VK_L);
        keyCodeMap.put("m", KeyEvent.VK_M);
        keyCodeMap.put("n", KeyEvent.VK_N);
        keyCodeMap.put("o", KeyEvent.VK_O);
        keyCodeMap.put("p", KeyEvent.VK_P);
        keyCodeMap.put("q", KeyEvent.VK_Q);
        keyCodeMap.put("r", KeyEvent.VK_R);
        keyCodeMap.put("s", KeyEvent.VK_S);
        keyCodeMap.put("t", KeyEvent.VK_T);
        keyCodeMap.put("u", KeyEvent.VK_U);
        keyCodeMap.put("v", KeyEvent.VK_V);
        keyCodeMap.put("w", KeyEvent.VK_W);
        keyCodeMap.put("x", KeyEvent.VK_X);
        keyCodeMap.put("y", KeyEvent.VK_Y);
        keyCodeMap.put("z", KeyEvent.VK_Z);
        keyCodeMap.put("0", KeyEvent.VK_0);
        keyCodeMap.put("1", KeyEvent.VK_1);
        keyCodeMap.put("2", KeyEvent.VK_2);
        keyCodeMap.put("3", KeyEvent.VK_3);
        keyCodeMap.put("4", KeyEvent.VK_4);
        keyCodeMap.put("5", KeyEvent.VK_5);
        keyCodeMap.put("6", KeyEvent.VK_6);
        keyCodeMap.put("7", KeyEvent.VK_7);
        keyCodeMap.put("8", KeyEvent.VK_8);
        keyCodeMap.put("9", KeyEvent.VK_9);
        keyCodeMap.put("`", KeyEvent.VK_BACK_QUOTE);
        keyCodeMap.put("-", KeyEvent.VK_MINUS);
        keyCodeMap.put("=", KeyEvent.VK_EQUALS);
        keyCodeMap.put("[", KeyEvent.VK_OPEN_BRACKET);
        keyCodeMap.put("]", KeyEvent.VK_CLOSE_BRACKET);
        keyCodeMap.put("\\", KeyEvent.VK_BACK_SLASH);
        keyCodeMap.put(";", KeyEvent.VK_SEMICOLON);
        keyCodeMap.put("\'", KeyEvent.VK_QUOTE);
        keyCodeMap.put(",", KeyEvent.VK_COMMA);
        keyCodeMap.put(".", KeyEvent.VK_PERIOD);
        keyCodeMap.put("/", KeyEvent.VK_SLASH);


        String chars = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./";
        String shiftChars = "~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";
        for(int i = 0; i < chars.length(); i++) {
            keyMap.put(chars.charAt(i), new String[]{chars.substring(i, i + 1)});
            keyMap.put(shiftChars.charAt(i), new String[]{"shift", chars.substring(i, i + 1)});
        }
        keyMap.put(' ', new String[]{"space"});
        keyMap.put('\t', new String[]{"tab"});
    }

    public static void setCustomizedShortcut(String name, String... keys) {
        customizedMap.put(name, keys);
    }

    public static void main(String[] args) {
        Tester.sleep(3);
        Tester.typeKeys("    ");
    }
}
