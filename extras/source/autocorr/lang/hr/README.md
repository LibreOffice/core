# Pravila za automatsko ispravljanje, iznimke kraja rečenice te iznimke u pisanju velikog i malog slova iz grafostilističkih razloga
Za LibreOfficeovu značajku automatskog ispravljanja važne su datoteke

* DocumentList.xml
* SentenceExceptList.xml
* WordExceptList.xml.

i svaka će od njih biti ukratko opisana u nastavku.

Budući da se ovaj popis ažurira češće nego što se sâm popis dodaje u izvorni kôd LibreOfficea, upute o tome kako ručno unijeti noviji popis u vaše računalo radi korištenja uz stariju inačicu LibreOfficea pogledajte u datoteci '[umetanje-automatskih-ispravaka.md](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/blob/master/umetanje-automatskih-ispravaka.md)'.

Opisi su datoteka (šturo) dokumentirani na

[https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras](https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras)


## DocumentList.xml
Datoteka sadrži popis za automatsko ispravljanje riječi ili određenih kombinacija znakova s novom riječi ili novom kombinacijom znakova.

Kombinacija znakova :^2: može poslužiti kao dobar primjer. Računalo će automatski zamijeniti ovu kombinaciju znakova sa drugom potencijom. Recimo X:^2: će dati X². [[Mogući ispravci](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/blob/master/DocumentList.xml)]

U ovu je datoteku poželjno dodavati i one ispravke koji su **nedvojbeno** zatipci.

U hrvatskom se umjesto infinitiva piše krnji infinitiv ako iza njega slijedi nenaglašeni oblik pomoćnog glagola htjeti: 'tražit ću', ne 'tražiti ću'. Nije dobro u ovu datoteku unositi takve ispravke jer, kako se radi o čestoj pogrešci, netko možda upravo o toj pogrešci i piše, recimo jezični savjetnik ili slično. Računalo bi svaki takav primjer moglo automatski ispraviti što bi rezultiralo uništenim tekstom. U tom slučaju autor **želi** 'tražiti ću' i ne želi da LibreOffice to automatski ispravlja bez ikakva upozorenja i podcrtavanja. Slično može biti i s primjerima poput 'avijon', 'idijot', 'kemia'...

Uvijek treba biti oprezan i izbjegavati da računalo izvršava radnje bez eksplicitne potvrde korisnika. Zadaća je računalne provjere pravopisa i gramatike da ispravljaju prethodno navedene primjere i da korisnika na njih upozore podcrtavanje, ali isto tako korisnik mora odlučiti želi li podcrtano ispravljati ili ne!

U ovu datoteku **treba** dodati ono što je nedvojbeno zatipak i ono što bi nepotrebno opterećivalo rječnik za provjeru pravopis ili povećavalo broj pravila u provjeri gramatike, recimo primjere poput 'pokuša tću' kako bi LibreOffice to automatski ispravio u 'pokušat ću' jer nema potrebe da korisnik gubi vrijeme na potvrđivanje takvih prijedloga.

Ali i tu treba biti umjeren!

U engleskoj se inačici pravila za automatsko ispravljanje nalazi pravilo koje zatipak 'nwe' zamjenjuje sa 'new', što je se na prvi pogled čini dobrim rješenjem i naoko je u skladu s ovjde iznesenim. Međutim, jedan je korisnik automatskog ispravljanja za engleski jezik trebao i **želio** napisati 'nwe' jer je to kratica (oznaka) za jedan mali jezik u Kamerunu (ili negdje oko Kameruna). LibreOffice je svaki 'nwe' zamijenio s 'new' bez ikakvog upozorenja i korisnik je primijetio grešku kada je već bilo kasno. Štetu nije moguće zamjenom (engl. find and replace) 'new' sa 'nwe' jer bi se promjena izvršila i na onim mjestima gdje doista i treba stajati 'new'.

U ovom bi slučaju podcrtavanje pomoću provjere pravopisa ili gramatike bilo bolje rješnje jer bi korisnik bio upozoren na potencijalnu pogrešku, ali sadržaj ne bi bio uništen neželjenim ispravljanjem.

Korisnik ne može znati što se sve nalazi na ovom popisu i onaj tko sastavlja popis nikada ne može predvidjeti sve moguće upotrebe i korištenja popisa pa onda i treba biti oprezan s onime što se na njega dodaje. Bolje je obrisati ili nedodatni nego dodatni pogrešno.

---

Popis se zamjena koje su dodani mimo inicijalnih/standardnih nalazi u datoteci '[dodano.txt](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/blob/master/dodano-na-documentlist.txt)'.


## SentenceExceptList.xml
Točka označava kraj rečenice. LibreOffice automatski riječ iza točke – početak nove rečenice – piše velikim slovom. Međutim, nije svaka točka i znak za kraj rečenice. Problem su kratice i skraćenice iza kojih se piše točka. Takva je kratica često dio rečenice, ne njezin kraj.

Da bi se spriječilo automatsko ispravljanje malog slova u veliko iza kratica, na ovaj popis treba dodati kratice koje se ne mogu nalaziti na samom kraju rečenice, ili se tamo nalaze vrlo rijetko.

Jasnije će biti na primjeru 'Tako je 2016. god. odlučeno [...]'. Čim korisnik napiše 'odlučeno', LibreOffice će automatski ispraviti 'odlučeno' u 'Odlučeno': 'Tako je 2016. god. Odlučeno [...]'.

Ali u ovom slučaju 'god.' ne označava kraj rečenice nego je skraćenica koja pripada rečenici i ne označava njezin kraj.

Treba biti oprezan i ne uključivati kratice koje su izrazom jednake riječima koje doista i mogu stajati na kraju rečenice: 'čest.' od 'čestica' i 'čest' kao pridjev koji doista i može stajati na kraju rečenice pa onda iza točke i počinje nova rečenica i sljedeću riječ treba pisati velikim slovom.

Potrebno je vagati i provjeravati u jezičnim korpusima. Pitanje je javlja li se češće riječ 'med' na kraju rečenice kao imenica 'Kupio je med.' ili je češće riječ o skraćenici koja ne označava kraj rečenice, recimo 'med.' za 'medicina'.

Iza rednih brojeva LibreOffice ne ispravlja mala slova.

### Dodatne napomene
Zanimljivo je to što kratica koja se nalazi na ovom popisu negira kraj rečenica čak i ako prethodna riječ nije na popisu kratica.

Na popisu se ne nalazi kratica 'dr.' pa je za očekivati je da će kratica 'sc.' u primjeru 'dr. sc. Marko Markić' biti automatski ispravljena u 'Sc.': 'dr. Sc. Marko Markić' – ali to nije slučaj.

Ako se iza kratice 'dr.' (nije na popisu) odmah doda kratica 'sc.' (jest na popisu), ova druga neće biti ispravljena u 'Sc.' jer izgleda da upravo **ona** negira kraj rečenice.

Pogleda li se primjer 'Ovo je kraj. sc. samo radi probe.' vidjet će se da LibreOffice neće automatski ispravljati malo slovo ni u 'sc.' (iza 'kraj.') ni u 'samo' (iza 'sc.') jer sada LibreOffice više ne vidi točku ni iza 'kraj' ni iza 'sc' kao oznaku kraja rečenice.

Ovo omogućuje da se (do jedne mjere) na popis unose i višedjelne kratice. Korisnik na ovaj popis kroz izbornik 'Alati → Automatsko ispravljanje → Mogućnosti automatskog ispravljanja' u kartici 'Iznimke' ne može dodati 'nar. pj.' s razmakom, a to treba pisati s razmakom.

Moguće je iskoristiti što je prethodno izneseno pa na popis dodati samo 'pj.' i kada korisnik napiše 'nar. pj.', mehanizam će automatskog ispravljanja raditi ono što je očekivano iako je ovo zapravo zaobilazni put.

Važno je ovo znati jer se u hrvatskom kratice uglavnom pišu s razmacima: [pravila o pisanju kratica](http://pravopis.hr/pravilo/kratice/49/).

Nije jednostavno odrediti čvrste kriterije po kojima bi se jednostavno moglo odlučiti koju kraticu uključiti a koju ne. Neke se kratice mogu i ne moraju nalaziti na kraju rečenice. Neke su kratice (metor. od meteorološki ili med. od medicinski) izrazom jednake drugim rječima (med, meteor) i tu treba biti oprezan. Ne preostaje drugo nego se voditi vlastitim jezičnim osjećajem i intuicijom.

Valjalo bi u računalnom korpusu hrvatskog jezika stvarno i provjeriti što se i kada se javlja češće i prema tome korigirati popis.

Na kraju se popisa nalaze kratice koje su uzete u obzir, ali nisu uključene na aktivan popis. Čisto da se može usporediti i da se vidi je li se o nekoj kratici već razmišljalo i što se odlučilo.

### Komentari uz problematične kratice

#### g.
LibreOffice iza rednih brojeva ne ispravlja malo slovo u veliko. Tako je jer se redni brojevi pišu s točkom i u tom slučaju točka nije interpunkcijski nego pravopisni znak. Nevolja je u tome što se brojevi ponekad mogu naći i na samom kraju rečenice i tada točka stoji i kao pravopisni znak (redni broj) i kako interpunkcijski znak (kraj rečenice). Korisnik će sâm morati prvu riječ sljedeće rečenice pisati velikim slovom i tu se ne može puno napraviti.

Odlučeno je da se na aktivni popis doda i kratica 'g.' iako se dosta često može nalaziti na kraju rečenice, često se i ne nalazi na kraju rečenice: 'U 2016. g. nismo uspjeli [...]'. Kao i sa rednim brojevima, ni ovdje nije moguće utvrditi označava li točka kraj rečenice ili jednostavno stoji uz kraticu. Budući da je dodana na ovaj popis, čak kada i bude na kraju rečenice, korisnik će sam morati prvu riječ sljedeće rečenice napisati velikim slovom, neće se provoditi automatsko ispravljanje.

Uvrštavanjem se ove kratice na popis ne kvari funkcionalnost (u najgorem je slučaju ostala ista), dapače popravlja se u onim situacijama kada 'g.' stoji za 'gospodin' i tada se nikad ne nalazi na kraju rečenice nego se očekuje da iza nje slijedi ime.

Iako nekome može pasti napamet da se posluži ovim mehanizmom kako ime iza kratice 'g.' (gospodin) ne bi morao sam pisati velikim slovom, to nije dobro jer lomljenje jedne rečenice na dvije, koje to zapravo nisu, može prevariti provjeru gramatike i time onemogućiti provjeru reda riječi, pisanje zareza ili slično.

Uzme li se u obzir da 'g.' stoji iza 'godina' i za 'gospodin', bolje ju je uključiti nego izostaviti, bez obzira što se u (rijetkim) slučajevima može nalaziti na kraju rečenice.


## WordExceptList.xml
Datoteka WordExceptList.xml sadrži pravila za automatsko ispravljanje riječi koje su pogrešno napisane dvama velikim slovima na početku riječi: MArko > Marko.

Ima primjera, ponajviše kratica i skraćenica, koje i treba pisati dvama velikim slovima. Kratica 'OOo' stoji za 'OpenOffice.org'. Kada 'OOo' ne bi bio na ovom popisu, LibreOffice bi 'OOo' ispravljao u 'Ooo'.

Međutim, izgleda da ima i odstupanja. LibreOffice nije ispravljao kraticu 'MHz' u 'Mhz' bez obzira što se nije nalazila na ovom popisu i to je zato što se ta kratica nalazila u rječniku za provjeru pravopisa. Znači da se ovakvo automatsko ispravljanje provodi samo na riječima koje se ne nalaze u rječniku za provjeru pravopisa ili se ne nalaze na ovom popisu. Za potvrdu teze može se izmisliti kratica 'MHt' i LibreOffice će je ispraviti u 'Mht', ali ako se kratica doda u rječnik za provjeru pravopisa ili na ovaj popis, automatsko ispravljanje izostaje.

Postoji samo problem u primjerima poput 'PC-a', 'PC-u' i sl. Ako se ove kratice napišu kako je prethodno prikazano - zapravo točno, onako kako je opisano u rječniku za provjeru pravopisa - problema nema. Ako se te kratice napišu pogrešno, kao 'PCa' ili 'PCja' ili 'PCu', 'PCem', 'PCjem' i slično, LibreOffice će automatski takve unose zamijeniti s 'Pca', 'Pcja', 'Pcu', 'Pcem', 'Pcjem' i slično. Sada je problem jer se rječniku za provjeru pravopisa šalju pogrešne riječi na provjeru, sada rječnik za provjeru pravopisa ne može predložiti 'PC-a' za 'PCa' nego predlaže 'Oca', 'Pac' (pâc), 'Psa' itd.

Zaobilazni je put da se na ovaj popis dodaju i najčešće pravopisne pogreške koje imaju dva velika slova na početku. Da bi se zaobišao ovaj nedostatak i da bi se dozvolilo rječniku za provjeru pravopisa da radi svoj posao, na popis je potrebno dodati primjere poput 'PCa', 'PCja', 'PCu', 'PCju', 'PCom', 'PCjom' i slično. Sada računalo neće više ispravljati 'PCa' u 'Pca' nego će ostaviti 'PCa', a rječnik će za provjeru pravopisa sada moći predložiti 'PC-a' kao ispravak.


## dodano-na-documentlist.txt
U datoteci se navode unosi kojih nema na izvornom LibreOfficeovom popisu. Dodani su naknadno što je bilo potrebno radi lakšeg ispravljanja eventualnih pogrešaka i vođenja evidencije o tome što je i kada je dodano na popis.


## Umetanje-automatskih-ispravaka.md
U datoteci se opisuje postupak unošenja ovog popisa za automatsko ispravljanje u računalo. Često su pravila za automatsko ispravljanje iz [ovog repozitorija](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/) novija od onih koja se nalaze u izvornom kodu LibreOfficea.
