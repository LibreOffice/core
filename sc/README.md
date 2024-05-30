# Spreadsheet Application Code

You can dump some information in a dbgutil build:

## Shortcuts

### CTRL+SHIFT+F12

Dumps the column width of the first 20 columns.

### CTRL+SHIFT+F11

Dumps the graphic objects and their position and size in pixel.

### CTRL+SHIFT+F6

Dumps the SfxItemSet representing the cell properties' of the
current selection as a xml file. The file will be named dump.xml

## The Cache Format

ScDocument::StoreTabToCache allows storing the content (not the formatting)
of a table to a binary cache format.

The format is column orientated which allows quick serialization of the table.

* Header:
    * Number of Columns: 64 bit unsigned integer

* Column:
    * Column Index: 64 bit unsigned integer
    * Column Size: 64 bit unsigned integer
    * For each cell type block a new ColumnBlock

* ColumnBlock:
    * Start Row: 64 bit unsigned integer
    * Block Size: 64 bit unsigned integer
    * Type: 8 bit unsigned integer
        - 0 : empty
        - 1 : numeric
            * for each cell: 64 bit IEEE 754 double precision value
        - 2 : string
            * for each cell: 32 bit signed string length followed by string length bytes of the string (UTF-8)
        - 3 : formula
            * for each cell: 32 bit signed string length followed by the formula in R1C1 notation as a string

## Functions Supporting Wildcards or Regular Expressions

As this comes up every now and then, and rather should be documented in an
extra list of the Help system, functions that support Wildcards or Regular
Expressions *and* depend on the setting under
Tools -> Options -> Calc -> Calculate are those that in ODF OpenFormula (ODFF)
are defined to depend on the HOST-USE-REGULAR-EXPRESSIONS or HOST-USE-WILDCARDS
properties, see ODF v1.3 part4 3.4 Host-Defined Behaviors
https://docs.oasis-open.org/office/OpenDocument/v1.3/os/part4-formula/OpenDocument-v1.3-os-part4-formula.html#__RefHeading__1017868_715980110

* Database Functions
  * DAVERAGE
  * DCOUNT
  * DCOUNTA
  * DGET
  * DMAX
  * DMIN
  * DPRODUCT
  * DSTDEV
  * DSTDEVP
  * DSUM
  * DVAR
  * DVARP
* Information Functions
  * COUNTIF
  * COUNTIFS
* Lambda Functions
  * LET
* Lookup Functions
  * HLOOKUP
  * LOOKUP
  * MATCH
  * XMATCH
  * VLOOKUP
  * XLOOKUP
  * FILTER
  * SORT
  * SORTBY
  * UNIQUE
* Mathematical Functions
  * RANDARRAY
  * SEQUENCE
  * SUMIF
  * SUMIFS
* Statistical Functions
  * AVERAGEIF
  * AVERAGEIFS
* Text Functions
  * SEARCH

