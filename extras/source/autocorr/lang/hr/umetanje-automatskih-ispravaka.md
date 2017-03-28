# Umetanje automatskih ispravaka na računalo
Vrlo je jednostavno dodati noviju inačicu pravila za automatsko ispravljanje u starije inačice LibreOfficea. Sve što treba napraviti jest preuzeti i preimenovati zip-datoteku i prenijeti je na odgovarajuće mjesto na računalo ovisno o tome koji se operacijski sustav koristi.

* preuzmite zip-datoteku iz repozitorija: [acor_hr-HR-LibreOffice](https://github.com/krunose/libo-acor-hr) ([izravna poveznica](https://github.com/krunose/libo-acor-hr/archive/master.zip))
* raspakirajte datoteku
* otvorite mapu koja je nastala raspakiravanjem datoteke
* arhivirajte sve datoteke iz te mape u zip-datoteku imena **acor_hr-HR.zip**
* preimenujte **zip** u **dat**. U konačnici ime datoteke mora biti **acor_hr-HR.dat**
* tako preimenovanu datoteku kopirajte
	* na **GNU/Linuxu** u datoteku **/home/&lt;user name&gt;/.config/libreOffice/4/user/autocorr**. Zamijenite &lt;user name&gt; vašim korisničkim imenom. Ako vam je korisničko ime za prijavu u sustav 'marko', onda datoteku treba kopirati u direktorij /home/marko/.config/libreOffice/4/user/autocorr. Staru acor_hr-HR.dat datoteku nemojte brisati, preimenujte je u 'backup-acor_hr-HR.dat za svaki slučaj
	* na **Windows** operacijskom sustavu u datoteku **%APPDATA%\libreoffice\4\user\autocorr** gdje %APPDATA% treba zamijeniti putanjom gdje se na vašem sustavu i inače instaliraju aplikacije. Najvjerojatnije **C:\Users&#92;&lt;user name&gt;\AppData\Roaming\libreoffice\4\user\autocorr**. Isto tako &lt;user name&gt; trebate zamijeniti vašim korisnički imenom. Ako vam je korisničko ime za prijavu u sustav 'marko' onda datoteku acor_hr-HR.dat treba kopirati u C:\Users\marko\AppData\Roaming\libreoffice\4\user\autocorr.
	* na **Mac OS X** operacijskom sustavu datoteku acor_hr-HR.dat treba kopirati u mapu **/Users/&lt;user name&gt;/Library/Application Support/LibreOffice/4/user/autocorr**. Zamijenite &lt;user name&gt; vašim korisnički imenom. Ako vam je korisničko ime za prijavu u sustav 'marko', datoteku treba kopirati u mapu /Users/marko/Library/Application Support/LibreOffice/4/user/autocorr.
* Ponovno pokrenite LibreOffice. Vodite računa da zatvorite i pokrenutu instanciju LibreOfficea u sistemskoj traci ako vam ja uključeno brzo pokretanje (engl. Quickstarter).

Ako niste sigurni u koju mapu trebate kopirati **acor_hr-HR.dat** datoteku, otvorite LibreOffice, u izborniku **Alati** odaberite **Mogućnosti**, s lijeve strane dijaloškog okvira odaberite stavku **LibreOffice** te ispod te kategorije odaberite **Putanje** i pogledajte koja putanja na desnoj strani stoji uz **Automatsko ispravljanje**.

Za više informacija ovašem korisničkom profilu povezanom s LibreOfficeom pogledajte na stranici [https://wiki.documentfoundation.org/UserProfile](https://wiki.documentfoundation.org/UserProfile). O putanjama se za automatsko ispravljanje govori u [točki 2.2 navedene stranice](https://wiki.documentfoundation.org/UserProfile#Default_location).

---

U ovu se datoteku mogu dodavati i vlastite kratice kroz izbornik **Alati** → **Automatsko ispravljanje** → **Mogućnosti automatskog ispravljanja...** U prvu se kućicu u kartici **Zamijeni** upisuje što se želi zamijeniti, a u drugu se kućicu upisuje čime se to želi zamijeniti. Zatim na desnoj strani dijaloškog okvira odaberite **Dodaj** i zatim potvrdite s **U redu**.

U istom se dijaloškom okviru mogu i brisati automatska ispravljanja.
