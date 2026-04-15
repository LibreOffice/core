# Latviešu valodas pareizrakstības pārbaudes bibliotēka (afiksu un vārdnīcas fails) 
# lietošanai ar OpenOffice 2.4.1 un augstāk
# Latvian spelling dictionary (affix and dictionary files) for OpenOffice 2.4.1 and higher
#
# Copyright (C) 2002-2020 Janis Eisaks, jancs@dv.lv, http://dict.dv.lv
# 
# Šī bibliotēka tiek licencēta ar Lesser General Public Licence (LGPL) 2.1 nosacījumiem. 
# Licences nosacījumi pievienoti failā license.txt vai iegūstami tīmekļa vietnē  
# http://www.fsf.org/licensing/licenses/lgpl.html
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# license along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


1. Uzstādīšana
2. Interesentiem
3. Izmaiņu saraksts

Svarīgs paziņojums:

šī vārdnīcas versija vairs neuztur MySpell pareizrakstības dziņa izmantošanu.
Afiksu bāze ir veidota, izmantojot Hunspell palašināto funkcionalitāti.

=================

1. Vārdīcas uzstādīšana

Ieteikums: uzstādīt vismaz OO 3.2 versiju.
Vārdnīcas uzstādīšana ir ļoti vienkārša - izmantojot OO Extension Manager.
Extension Manager piedāvā iespēju kā tiešsaistes, tā lokālu paplašinājumu uzstādīšanu.
Ja uzstādīšana tiešsaistes režīmā nav iespējama, vajadzīgo valodas paplašinājumu (vārdnīcu)
var lejupielādēt šeit:

http://extensions.services.openoffice.org/dictionary

un izmantot lokālai uzstādīšanai.

Ja izmantojat OO versiju, kas neuztur Extensions (pirms 2.4.1), tad:

1. iespēja. Uzstādīšana tiešsaistes režīmā
 No izvēlnes File/Wizards/Install new dictionaries palaidiet att. vedni, izvēlieties 
 Jums tīkamo vedņa valodu un sekojiet norādījumiem. Bez latviešu valodas pareizrakstības 
 rīkiem Jūs vienlaicīgi varat uzstādīt papildus valodas vai atsvaidzināt esošās bibliotēkas.
 (Uzmanību! - nav zināms, cik ilgi šī bibliotēka vēl tiks aktualizēta; pilns laidienu arhīvs ir 
  atrodams http://sourceforge.net/projects/openoffice-lv/)

 Ja kaut kādu iemeslu dēļ nevarat izmantot 1. iespēju, ir
 
 2. iespēja. "Offline" uzstādīšana
 Lejupielādējiet pēdējo moduļa versiju no openoffice-lv.sourceforge.net .
 Pēc faila iegūšanas tas ir jāatpako direktorijā %Openoffice%\share\dict\ooo, 
 kur %Openoffice% - direktorija, kurā veikta OpenOffice uzstādīšana. Tur esošajam failam 
 dictionary.lst ir jāpievieno sekojošas rindas: 
 
 DICT lv LV lv_LV
 HYPH lv LV hyph_lv_LV

 vai arī jāizpilda win-lv_LV_add.bat (Windows gadījumā) vai, Linux gdījumā, jāizpilda 
 komandu:

   sh <lin-lv_LV_add.sh

 Lai izpildītu 2. iespēju, Jums ir jābūt tiesībām rakstīt minētajā katalogā. Ja tādu nav, 
 varat uzstādīt vērdnīcu lokāli, savā lietotāja opciju katalogā (%OOopt%/user/wordbook).

 Offline uzstadīšanai var izmantot arī 1. iespējā minēto vedni, viss notiks līdzīgi, 
 tikai nepieciešamajām moduļu pakotnēm būs jābūt uz lokālā diska. Jāpiezīmē ka, piemēram, 
 SUSE gadījumā minētais vednis ir izgriezts ārā no OO un 2. iespeja ir vienīgā. Atsevišķi 
 šis līdzeklis un vārdnīcas ir iegūstams tīmekļa vietnē
 
  http://wiki.services.openoffice.org/wiki/Dictionaries

Ar to moduļu uzstādīšana praktiski ir pabeigta; atliek vienīgi caur 
Options>Language settings>Writing aids ieslēgt vai izslēgt nepieciešamos moduļus un 
iestatīt dokumentu noklusēto valodu.


 Ja ir nepieciešama automātiskā pareizrakstības pārbaude, zem Tools>Spellcheck jāieķeksē 
 AutoSpellcheck.

================

2. Interesentiem

Ja jums ir iekrājušies vārdi, kurus šis līdzeklis neatpazīst vai arī atpazīst kļūdaini, esat
laipni aicināti tos atsūtīt tālākai vārdnīcas pilnveidošanai vai arī reģistrēties vārdnīcas 
izstrādei veltītajā vietnē //dict.dv.lv.

Lielāka apjoma dokumentu filtrēšanai var izmantot sekojošā vietā atrodamu StarBasic makrosu:
http://lingucomponent.openoffice.org/servlets/ReadMsg?listName=dev&msgNo=1843

Piezīme - makross nedarbojas ar OO >3.0.

Sarakstu gadījumā ir lūgums sākumā pašiem kritiski izvērtēt neatpazīto vārdu pareizību 
vai to pielietojamību (piem slengs, barbarismi utml. drazas, manuprāt, nav tā vērtas, 
lai tās iekļautu pareizrakstības pārbaudes vārdnīcā, lai gan viena otra tomēr iespraucas).
