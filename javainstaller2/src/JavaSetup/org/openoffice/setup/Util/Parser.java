package org.openoffice.setup.Util;

public class Parser {

    private Parser() {
    }

    static public boolean parseBoolean(String input) {
        boolean value;
        if ( input.equalsIgnoreCase("true")) {
            value = true;
        } else if ( input.equalsIgnoreCase("false")) {
            value = false;
        } else {
            value = false;
            System.err.println("Cannot convert String to boolean: " + input);
            System.exit(1);
        }
        return value;
    }

}
