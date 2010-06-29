These Files belong to the project at:
http://wiki.services.openoffice.org/wiki/Mnemonics_Localisation

' Raffaella Braconi and her l10n team defined these reference files;
' If there is a difference, it is a bug, that has to get defined by her.
' Or checked on the wiki page.

It belongs to the test qa/qatesttool/framework/level1/f_lvl1_menuentries.bas
Which generates the files for a jet unknown language.

Mnemonics Localisation
To try to solve the problem of duplicate or unusual mnemonics in the product the StarOffice QA team has created a testtool script which is able to detect changes in the mnemonics menu entries which may erroneously occur during translation.
The menu items (first level) should have - as far as possible - the same mnemonics for both consistency and usability reasons.
Automating the mnemonic testing it means that initially we would need to invest some time in verifying and, if necessary fix, the mnemonics in the main menus. Based on that information the script would be able to tell us if and what has been changed from version to version.
Thorsten B. from the StarOffice QA team, has now created text documents containing the menu entries with the mnemonics for
de, en-us, es, fr, hu, it, nl, pl, pt-br, ru, sv
based on the OOC680m0 build. 
...
